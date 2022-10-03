#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "load_bpf.h"
#include "xdp.skel.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "bpf/xdp.bpf.h"
#include "ovs-ofctl.h"
#include "command-line.h"
#include "openvswitch/ofp-flow.h"
#include "balancer.h"
#include "hashmap.h"
#include "ringbuffer.h"

uint8_t looping = 1;

void add_flow(int in_port, int out_port, int proto, uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port){
    char str[150];
    char src_ip_str[15];
    char dst_ip_str[15];
    sprintf(src_ip_str, "%d.%d.%d.%d", src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF);
    sprintf(dst_ip_str, "%d.%d.%d.%d", dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF);
    sprintf(str, "in_port=%d,ip,dl_type=0x0800,nw_proto=%d,nw_src=%s,nw_dst=%s,tp_src=%u,tp_dst=%u,priority=1,actions=output:%d", 
        in_port,
        proto,
        src_ip_str,
        dst_ip_str,
        src_port,
        dst_port,
        out_port);
    custom_add_flow(str);
}

void del_flow(int in_port, int out_port, int proto, uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port){
    char str[150];
    char src_ip_str[15];
    char dst_ip_str[15];
    sprintf(src_ip_str, "%d.%d.%d.%d", src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF);
    sprintf(dst_ip_str, "%d.%d.%d.%d", dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF);
    sprintf(str, "in_port=%d,ip,dl_type=0x0800,nw_proto=%d,nw_src=%s,nw_dst=%s,tp_src=%u,tp_dst=%u", 
        in_port,
        proto,
        src_ip_str,
        dst_ip_str,
        src_port,
        dst_port);
    custom_del_flow(str);
}

void init_flows(){
    char str[50];
    custom_add_flow("arp,actions=FLOOD");
    sprintf(str, "ip,in_port=%d,priority=0,actions=output:%d", RX_SWITCH_IFINDEX, TX_SWITCH_IFINDEX);
    custom_add_flow(str);
    sprintf(str, "ip,in_port=%d,priority=0,actions=output:%d", TX_SWITCH_IFINDEX, RX_SWITCH_IFINDEX);
    custom_add_flow(str);
}

void cleanup_flows(){
    custom_del_flows();
}

uint64_t get_flow_stats(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port){
    char config_spec[100];
    sprintf(config_spec, "in_port=%d,ip,dl_type=0x0800,nw_proto=%d,nw_src=%d.%d.%d.%d,nw_dst=%d.%d.%d.%d,tp_src=%u,tp_dst=%u", 
        RX_SWITCH_IFINDEX,
        6,
        src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF,
        dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF,
        src_port,
        dst_port);
    struct ofputil_flow_stats *fses;
    size_t n_fses;
    custom_dump_flows(config_spec, &fses, &n_fses);
    uint64_t packets = fses[0].packet_count;
    free_dump(fses, n_fses);
    return packets;
}

void handle_interrupt(int sig) {
    cleanup_flows();
    printf("Caught interrupt signal, exiting...\n");
    looping = 0;
}

void collect_map(struct FiveTuple key, int map_fd, struct ConnectionState *value){
    int nb_cpus = libbpf_num_possible_cpus();
    struct ConnectionState state[nb_cpus];
    bpf_map_lookup_elem(map_fd, &key, &state);
    for (int i = 0; i < nb_cpus; i++){
        if (state[i].SYN)
            value->SYN = 1;
        if (state[i].SYNACK)
            value->SYNACK = 1;
        if (state[i].ACK)
            value->ACK = 1;
        if (state[i].FIN)
            value->FIN = 1;
        if (state[i].HANDLED)
            value->HANDLED = 1;
    }
}

void mark_handled(struct FiveTuple key, int map_fd){
    int nb_cpus = libbpf_num_possible_cpus();
    struct ConnectionState state[nb_cpus];
    bpf_map_lookup_elem(map_fd, &key, &state);
    for (int i = 0; i < nb_cpus; i++){
        state[i].SYN = 1;
        state[i].SYNACK = 1;
        state[i].ACK = 1;
        state[i].HANDLED = 1;
    }
    bpf_map_update_elem(map_fd, &key, &state, BPF_ANY);
}

void mark_fin(struct FiveTuple key, int map_fd){
    int nb_cpus = libbpf_num_possible_cpus();
    struct ConnectionState state[nb_cpus];
    bpf_map_lookup_elem(map_fd, &key, &state);
    for (int i = 0; i < nb_cpus; i++){
        state[i].FIN = 1;
    }
    bpf_map_update_elem(map_fd, &key, &state, BPF_ANY);
}

int user_space_prog(int connections_map_fd){
    // Contains the user space logic
    // Typically, should be a forever looping program
    // First, allow ARP flooding
    init_flows();
    struct HashMap *map = hashmap_init();
    while (looping)
    {
        struct FiveTuple prev_key = {};
        struct FiveTuple key;
        struct FiveTuple key_to_delete[HASHMAP_SIZE];
        int key_to_delete_index = 0;
        // Check BPF maps
        while(bpf_map_get_next_key(connections_map_fd, &prev_key, &key) == 0){
            struct ConnectionState state = {0};
            collect_map(key, connections_map_fd, &state); 
            if (!state.HANDLED){
                add_flow(RX_SWITCH_IFINDEX, TX_SWITCH_IFINDEX, key.proto, key.src_ip, key.dst_ip, key.src_port, key.dst_port);
                // Add entry to hashmap
                hashmap_new(map, &key);
                mark_handled(key, connections_map_fd);
            }
            struct RingBuffer *ring_buffer = hashmap_get(map, &key);
            if (ring_buffer){
                uint64_t last = ringbuffer_get_last(ring_buffer);
                ringbuffer_add(ring_buffer, get_flow_stats(key.src_ip, key.dst_ip, key.src_port, key.dst_port));
                if (ringbuffer_is_flat(ring_buffer)){
                    // Mark the connection as terminated
                    mark_fin(key, connections_map_fd);
                    // Remove entry from hashmap
                    hashmap_remove(map, &key);
                }
            }
            prev_key = key;
        }
        for (int i = 0; i < key_to_delete_index; i++){
            bpf_map_delete_elem(connections_map_fd, &key_to_delete[i]);
        }
        // Balance flows
        struct Migrations migrations = {0};
        balancer_balance(map, 8, &migrations);
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