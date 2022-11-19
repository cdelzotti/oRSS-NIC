#include <sys/socket.h>
#include <netinet/in.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "load_bpf.h"
#include "xdp.skel.h"
#include "bpf/xdp.bpf.h"
#include "ovs-ofctl.h"
#include "command-line.h"
#include "openvswitch/ofp-flow.h"
#include "balancer.h"
#include "hashmap.h"
#include "ringbuffer.h"
#include "openflow.h"

uint8_t looping = 1;

void add_flow(int in_port, int out_processor, uint8_t proto, uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port){
    char str[150];
    char src_ip_str[15];
    char dst_ip_str[15];
    sprintf(src_ip_str, "%d.%d.%d.%d", src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF);
    sprintf(dst_ip_str, "%d.%d.%d.%d", dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF);
    sprintf(str, "in_port=%d,ip,dl_type=0x0800,nw_proto=%u,nw_src=%s,nw_dst=%s,tp_src=%u,tp_dst=%u,priority=1,actions=resubmit(,%d)", 
        in_port,
        proto,
        src_ip_str,
        dst_ip_str,
        src_port,
        dst_port,
        out_processor + 1);
    custom_add_flow(str);
}

void del_flow(int in_port, uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t proto){
    char str[150];
    char src_ip_str[15];
    char dst_ip_str[15];
    char proto_str[3];
    sprintf(src_ip_str, "%d.%d.%d.%d", src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF);
    sprintf(dst_ip_str, "%d.%d.%d.%d", dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF);
    sprintf(str, "table=0,nw_proto=%u,in_port=%d,nw_src=%s,nw_dst=%s,tp_src=%u,tp_dst=%u", 
        proto,
        in_port,
        src_ip_str,
        dst_ip_str,
        src_port,
        dst_port);
    custom_del_flow(str);
}

void init_flows(int nbCores){
    char str[100];
    // Allow ARP
    custom_add_flow("arp,actions=FLOOD");
    // Allow host to send packets without check
    sprintf(str, "in_port=%d,priority=0,actions=output:%d", TX_SWITCH_IFINDEX, RX_SWITCH_IFINDEX);
    custom_add_flow(str);
    // Set VLAN pushing rules
    for(int i = 0; i < nbCores + 1; i++){
        sprintf(str, "ip,in_port=%d,table=%d,priority=0,actions=mod_vlan_vid:%d,output:%d", RX_SWITCH_IFINDEX, i+1, i, TX_SWITCH_IFINDEX);
        // sprintf(str, "ip,in_port=%d,table=%d,priority=0,actions=output:%d", RX_SWITCH_IFINDEX, i+1, TX_SWITCH_IFINDEX);
	custom_add_flow(str);
    }
    // On receiving side,need to push a vlan tag
    sprintf(str, "ip,in_port=%d,priority=0,actions=resubmit(,1)", RX_SWITCH_IFINDEX);
    custom_add_flow(str);
}

void cleanup_flows(){
    custom_del_flows();
}

uint64_t get_flow_stats(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t proto){
    char config_spec[100];
    sprintf(config_spec, "in_port=%d,ip,dl_type=0x0800,nw_proto=%u,nw_src=%d.%d.%d.%d,nw_dst=%d.%d.%d.%d,tp_src=%u,tp_dst=%u", 
        RX_SWITCH_IFINDEX,
        proto,
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
    // cleanup_flows();
    printf("\nCaught interrupt signal, shutting connection...\n");
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

void apply_migrations(struct Migrations *migrations){
    char str[125];
    char proto_str[4];
    for (int i = 0; i < migrations->nb_migrations; i++){
        struct Migration migration = migrations->migrations[i];
        if (migration.key.proto == 6)
            strcpy(proto_str, "tcp");
        else
            strcpy(proto_str, "udp");
        sprintf(str, "table=0,in_port=%d,nw_src=%d.%d.%d.%d,nw_dst=%d.%d.%d.%d,%s,tp_src=%u,tp_dst=%u,actions=resubmit(,%d)",
            RX_SWITCH_IFINDEX,
            migration.key.src_ip & 0xFF, (migration.key.src_ip >> 8) & 0xFF, (migration.key.src_ip >> 16) & 0xFF, (migration.key.src_ip >> 24) & 0xFF,
            migration.key.dst_ip & 0xFF, (migration.key.dst_ip >> 8) & 0xFF, (migration.key.dst_ip >> 16) & 0xFF, (migration.key.dst_ip >> 24) & 0xFF,
            proto_str,
            migration.key.src_port,
            migration.key.dst_port,
            migration.destination_core + 1);
        custom_mod_flow(str);
    }
}

int user_space_prog(int connections_map_fd){
    // Contains the user space logic
    // Typically, should be a forever looping program
    // First, allow ARP flooding
    init_flows(NB_CORES);
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
            struct RingBuffer *ring_buffer = hashmap_get(map, &key);
            collect_map(key, connections_map_fd, &state); 
            if (!state.HANDLED){
                // Add entry to hashmap
                printf("Adding entry to hashmap\n");
                hashmap_new(map, &key);
                ring_buffer = hashmap_get(map, &key);
                add_flow(RX_SWITCH_IFINDEX, ring_buffer->assigned_core, key.proto, key.src_ip, key.dst_ip, key.src_port, key.dst_port);
                mark_handled(key, connections_map_fd);
            }
            if (ring_buffer){
                uint64_t last = ringbuffer_get_last(ring_buffer);
                ringbuffer_add(ring_buffer, get_flow_stats(key.src_ip, key.dst_ip, key.src_port, key.dst_port, key.proto));
                if (ringbuffer_is_timedout(ring_buffer)){
                    // Remove flow
                    printf("Flow is timing out\n");
                    del_flow(RX_SWITCH_IFINDEX, key.src_ip, key.dst_ip, key.src_port, key.dst_port, key.proto);
                    // Add key to delete
                    key_to_delete[key_to_delete_index] = key;
                    key_to_delete_index++;
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
        balancer_balance(map, NB_CORES, &migrations);
        apply_migrations(&migrations);
        sleep(1);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    // // Initial setup
    // libbpf_set_print(libbpf_print_fn);
    // bump_memlock_rlimit();
    // signal(SIGINT, handle_interrupt);
    // cleanup_flows();
    // // Load BPF
    // struct xdp_bpf *skel = attach_bpf();
    // // Setup redirection map
    // int RX = RX_IFINDEX;
    // int TX = TX_IFINDEX;
    // int redir_map_fd = bpf_map__fd(skel->maps.map_redir);
    // int err = bpf_map_update_elem(redir_map_fd, &TX, &RX, 0);
    // if (err) {
    //     printf("Error setting up redirection map TX to RX: %s\n", strerror(err));
    //     return -1;
    // }
    // err = bpf_map_update_elem(redir_map_fd, &RX, &TX, 0);
    // if (err) {
    //     printf("Error setting up redirection map RX to TX: %s\n", strerror(err));
    //     return -1;
    // }
    // // Setup maps
    // int connections_fd = bpf_map__fd(skel->maps.connections);
    // if (connections_fd < 0) {
    //     fprintf(stderr, "Failed to get connections map FD: %s\n", strerror(errno));
    //     detach_bpf(skel);
    //     exit(1);
    // }
    // printf("Everything loaded correctly!\n");
    // user_space_prog(connections_fd);
    // // Detach BPF
    // detach_bpf(skel);
    // printf("Everything unloaded correctly!\n");
    openflow_connection ofp_connection;
    openflow_create_connection(&ofp_connection);
    signal(SIGINT, handle_interrupt);
    while (looping) {
        openflow_control(&ofp_connection);
        openflow_flows flows = {0};
        openflow_get_flows(&ofp_connection, &flows);
        printf("Got %u flows !\n", flows.nb_flows);
        openflow_free_flows(&flows);
    }
    // closing the listening socket
    openflow_terminate_connection(&ofp_connection);

    return 0;
}
