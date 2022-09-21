#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "load_bpf.h"
#include "xdp.skel.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "bpf/xdp.bpf.h"

#include "ovs_utils.h"

uint8_t looping = 1;

void add_flow(int in_port, int out_port, int proto, uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port){
    char str[200];
    char src_ip_str[20];
    char dst_ip_str[20];
    sprintf(src_ip_str, "%d.%d.%d.%d", src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF);
    sprintf(dst_ip_str, "%d.%d.%d.%d", dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF);
    sprintf(str, "ovs-ofctl -O OpenFlow12 add-flow %s in_port=%d,ip,dl_type=0x0800,nw_proto=%d,nw_src=%s,nw_dst=%s,tp_src=%u,tp_dst=%u,priority=1,actions=output:%d", 
        OVS_BRIDGE,
        in_port,
        proto,
        src_ip_str,
        dst_ip_str,
        src_port,
        dst_port,
        out_port);
    system(str);
}

void init_flows(){
    char str[100];
    sprintf(str, "ovs-ofctl -O OpenFlow12 add-flow %s arp,actions=FLOOD", OVS_BRIDGE);
    system(str);
    sprintf(str, "ovs-ofctl -O OpenFlow12 add-flow %s ip,in_port=%d,priority=0,actions=output:%d", OVS_BRIDGE, RX_SWITCH_IFINDEX, TX_SWITCH_IFINDEX);
    system(str);
    sprintf(str, "ovs-ofctl -O OpenFlow12 add-flow %s ip,in_port=%d,priority=0,actions=output:%d", OVS_BRIDGE, TX_SWITCH_IFINDEX, RX_SWITCH_IFINDEX);
    system(str);
}

void cleanup_flows(){
    char str[80];
    sprintf(str, "ovs-ofctl -O OpenFlow12 del-flows %s", OVS_BRIDGE);
    system(str);
}

int get_flow_stats(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port){
    char str[150];
    char src_ip_str[20];
    char dst_ip_str[20];
    sprintf(src_ip_str, "%d.%d.%d.%d", src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF);
    sprintf(dst_ip_str, "%d.%d.%d.%d", dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF);
    sprintf(str, "ovs-ofctl dump-flows %s ip,tcp,nw_src=%s,nw_dst=%s,tp_src=%u,tp_dst=%u", 
        OVS_BRIDGE,
        src_ip_str,
        dst_ip_str,
        (unsigned int) src_port,
        (unsigned int) dst_port);
    FILE *fp = popen(str, "r");
    char output[100];
    fgets(output, 100, fp);
    fgets(output, 100, fp);
    // Split on commas
    char *token = strtok(output, ",");
    int i = 0;
    while(token != NULL){
        if(i == 3){
            // Get the packet count
            char *token2 = strtok(token, "=");
            token2 = strtok(NULL, "=");
            return atoi(token2);
        }
        token = strtok(NULL, ",");
        i++;
    }

}

void handle_interrupt(int sig) {
    cleanup_flows();
    printf("Caught interrupt signal, exiting...\n");
    looping = 0;
}

int user_space_prog(int connections_map_fd){
    // Contains the user space logic
    // Typically, should be a forever looping program
    // First, allow ARP flooding
    init_flows();
    while (looping)
    {
        struct FiveTuple prev_key = {};
        struct FiveTuple key;
        printf("Printing connections map:\n");
        while(bpf_map_get_next_key(connections_map_fd, &prev_key, &key) == 0){
            struct ConnectionState state; 
            bpf_map_lookup_elem(connections_map_fd, &key, &state);
            int packets;
            if (state.ACK && state.SYN && state.SYNACK){
                printf("[ESTABLISHED] ");
                packets = get_flow_stats(key.src_ip, key.dst_ip, key.src_port, key.dst_port);
            } else if (state.FIN) {
                printf("[TERMINATED] ");
                packets = -1;
            } else {
                printf("[HANDSHAKING] ");
                packets = -1;
            }
            printf("%d.%d.%d.%d:%u -> %d.%d.%d.%d:%u ", 
                key.src_ip & 0xFF, (key.src_ip >> 8) & 0xFF, (key.src_ip >> 16) & 0xFF, (key.src_ip >> 24) & 0xFF, (unsigned int) key.src_port,
                key.dst_ip & 0xFF, (key.dst_ip >> 8) & 0xFF, (key.dst_ip >> 16) & 0xFF, (key.dst_ip >> 24) & 0xFF, (unsigned int) key.dst_port);
            prev_key = key;
            printf("(%d packets received)\n", packets);
            if (!state.HANDLED){
                // Add flow
                add_flow(RX_SWITCH_IFINDEX, TX_SWITCH_IFINDEX, key.proto, key.src_ip, key.dst_ip, key.src_port, key.dst_port);
                add_flow(TX_SWITCH_IFINDEX, RX_SWITCH_IFINDEX, key.proto, key.dst_ip, key.src_ip, key.dst_port, key.src_port);
                // Mark the connection as handled
                state.HANDLED = 1;
                bpf_map_update_elem(connections_map_fd, &key, &state, BPF_ANY);
            }
        }
        printf("-------------------------\n");
        sleep(1);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    // Initial setup
    libbpf_set_print(libbpf_print_fn);
    bump_memlock_rlimit();
    signal(SIGINT, handle_interrupt);
    cleanup_flows();
    // Load BPF
    struct xdp_bpf *skel = attach_bpf();
    // Setup redirection map
    int RX = RX_IFINDEX;
    int TX = TX_IFINDEX;
    int redir_map_fd = bpf_map__fd(skel->maps.map_redir);
    int err = bpf_map_update_elem(redir_map_fd, &TX, &RX, 0);
    if (err) {
        printf("Error setting up redirection map TX to RX: %s\n", strerror(err));
        return -1;
    }
    err = bpf_map_update_elem(redir_map_fd, &RX, &TX, 0);
    if (err) {
        printf("Error setting up redirection map RX to TX: %s\n", strerror(err));
        return -1;
    }
    // Setup maps
    int connections_fd = bpf_map__fd(skel->maps.connections);
    if (connections_fd < 0) {
        fprintf(stderr, "Failed to get connections map FD: %s\n", strerror(errno));
        detach_bpf(skel);
        exit(1);
    }
    printf("Everything loaded correctly!\n");
    user_space_prog(connections_fd);
    // Detach BPF
    detach_bpf(skel);
    printf("Everything unloaded correctly!\n");
    return 0;
}