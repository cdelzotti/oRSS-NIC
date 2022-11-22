#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "balancer.h"
#include "hashmap.h"
#include "ringbuffer.h"
#include "openflow.h"

uint8_t looping = 1;
uint8_t interrupted = 0;


void handle_interrupt(int sig) {
    // cleanup_flows();
    if (!interrupted){
        printf("\nCaught interrupt signal, shutting connection...\nPress Ctrl+C again to force exit\n");
        interrupted = 1;
        looping = 0;
    } else {
        printf("\nForcing exit...\n");
        exit(0);
    }
    looping = 0;
}

// uint64_t get_flow_stats(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t proto){
//     char config_spec[100];
//     sprintf(config_spec, "in_port=%d,ip,dl_type=0x0800,nw_proto=%u,nw_src=%d.%d.%d.%d,nw_dst=%d.%d.%d.%d,tp_src=%u,tp_dst=%u", 
//         RX_SWITCH_IFINDEX,
//         proto,
//         src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF,
//         dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF,
//         src_port,
//         dst_port);
//     struct ofputil_flow_stats *fses;
//     size_t n_fses;
//     custom_dump_flows(config_spec, &fses, &n_fses);
//     uint64_t packets = fses[0].packet_count;
//     free_dump(fses, n_fses);
//     return packets;
// }


// void collect_map(struct FiveTuple key, int map_fd, struct ConnectionState *value){
//     int nb_cpus = libbpf_num_possible_cpus();
//     struct ConnectionState state[nb_cpus];
//     bpf_map_lookup_elem(map_fd, &key, &state);
//     for (int i = 0; i < nb_cpus; i++){
//         if (state[i].SYN)
//             value->SYN = 1;
//         if (state[i].SYNACK)
//             value->SYNACK = 1;
//         if (state[i].ACK)
//             value->ACK = 1;
//         if (state[i].FIN)
//             value->FIN = 1;
//         if (state[i].HANDLED)
//             value->HANDLED = 1;
//     }
// }

// void mark_handled(struct FiveTuple key, int map_fd){
//     int nb_cpus = libbpf_num_possible_cpus();
//     struct ConnectionState state[nb_cpus];
//     bpf_map_lookup_elem(map_fd, &key, &state);
//     for (int i = 0; i < nb_cpus; i++){
//         state[i].SYN = 1;
//         state[i].SYNACK = 1;
//         state[i].ACK = 1;
//         state[i].HANDLED = 1;
//     }
//     bpf_map_update_elem(map_fd, &key, &state, BPF_ANY);
// }

// void mark_fin(struct FiveTuple key, int map_fd){
//     int nb_cpus = libbpf_num_possible_cpus();
//     struct ConnectionState state[nb_cpus];
//     bpf_map_lookup_elem(map_fd, &key, &state);
//     for (int i = 0; i < nb_cpus; i++){
//         state[i].FIN = 1;
//     }
//     bpf_map_update_elem(map_fd, &key, &state, BPF_ANY);
// }

// void apply_migrations(struct Migrations *migrations){
//     char str[125];
//     char proto_str[4];
//     for (int i = 0; i < migrations->nb_migrations; i++){
//         struct Migration migration = migrations->migrations[i];
//         if (migration.key.proto == 6)
//             strcpy(proto_str, "tcp");
//         else
//             strcpy(proto_str, "udp");
//         sprintf(str, "table=0,in_port=%d,nw_src=%d.%d.%d.%d,nw_dst=%d.%d.%d.%d,%s,tp_src=%u,tp_dst=%u,actions=resubmit(,%d)",
//             RX_SWITCH_IFINDEX,
//             migration.key.src_ip & 0xFF, (migration.key.src_ip >> 8) & 0xFF, (migration.key.src_ip >> 16) & 0xFF, (migration.key.src_ip >> 24) & 0xFF,
//             migration.key.dst_ip & 0xFF, (migration.key.dst_ip >> 8) & 0xFF, (migration.key.dst_ip >> 16) & 0xFF, (migration.key.dst_ip >> 24) & 0xFF,
//             proto_str,
//             migration.key.src_port,
//             migration.key.dst_port,
//             migration.destination_core + 1);
//         custom_mod_flow(str);
//     }
// }

// int user_space_prog(int connections_map_fd){
//     // Contains the user space logic
//     // Typically, should be a forever looping program
//     // First, allow ARP flooding
//     init_flows(NB_CORES);
//     struct HashMap *map = hashmap_init();
//     while (looping)
//     {
//         struct FiveTuple prev_key = {};
//         struct FiveTuple key;
//         struct FiveTuple key_to_delete[HASHMAP_SIZE];
//         int key_to_delete_index = 0;
//         // Check BPF maps
//         while(bpf_map_get_next_key(connections_map_fd, &prev_key, &key) == 0){
//             struct ConnectionState state = {0};
//             struct RingBuffer *ring_buffer = hashmap_get(map, &key);
//             collect_map(key, connections_map_fd, &state); 
//             if (!state.HANDLED){
//                 // Add entry to hashmap
//                 printf("Adding entry to hashmap\n");
//                 hashmap_new(map, &key);
//                 ring_buffer = hashmap_get(map, &key);
//                 add_flow(RX_SWITCH_IFINDEX, ring_buffer->assigned_core, key.proto, key.src_ip, key.dst_ip, key.src_port, key.dst_port);
//                 mark_handled(key, connections_map_fd);
//             }
//             if (ring_buffer){
//                 uint64_t last = ringbuffer_get_last(ring_buffer);
//                 ringbuffer_add(ring_buffer, get_flow_stats(key.src_ip, key.dst_ip, key.src_port, key.dst_port, key.proto));
//                 if (ringbuffer_is_timedout(ring_buffer)){
//                     // Remove flow
//                     printf("Flow is timing out\n");
//                     del_flow(RX_SWITCH_IFINDEX, key.src_ip, key.dst_ip, key.src_port, key.dst_port, key.proto);
//                     // Add key to delete
//                     key_to_delete[key_to_delete_index] = key;
//                     key_to_delete_index++;
//                     // Mark the connection as terminated
//                     mark_fin(key, connections_map_fd);
//                     // Remove entry from hashmap
//                     hashmap_remove(map, &key);
//                 }
//             }
//             prev_key = key;
//         }
//         for (int i = 0; i < key_to_delete_index; i++){
//             bpf_map_delete_elem(connections_map_fd, &key_to_delete[i]);
//         }
//         // Balance flows
//         struct Migrations migrations = {0};
//         balancer_balance(map, NB_CORES, &migrations);
//         apply_migrations(&migrations);
//         sleep(1);
//     }
//     return 0;
// }

void apply_migration(openflow_connection *ofp_connection, struct Migration *migration){
    openflow_mod_vlan(ofp_connection, &migration->key, migration->destination_core);
}

void get_migrations(openflow_flows *flows, struct HashMap *map, struct Migrations *migrations){
    for (int i=0; i<flows->nb_flows;i++){
        // Get FiveTuple key
        struct FiveTuple key = {0};
        key.src_ip = flows->flow_stats[i].match.nw_src;
        key.dst_ip = flows->flow_stats[i].match.nw_dst;
        key.src_port = flows->flow_stats[i].match.tp_src;
        key.dst_port = flows->flow_stats[i].match.tp_dst;
        key.proto = flows->flow_stats[i].match.nw_proto;
        // Get RingBuffers
        struct RingBuffer *ring_buffer = hashmap_get(map, &key);
        if (!ring_buffer){
            // If it does not exist, create it
            ring_buffer = hashmap_new(map, &key);
        }
        ringbuffer_add(ring_buffer, openflow_ovsbe64_to_uint64(flows->flow_stats[i].packet_count));
    }
    // Balance flows
    balancer_balance(map, NB_CORES, migrations);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, handle_interrupt);
    uint32_t loops = 0;
    struct HashMap *map = hashmap_init();
    // Create OpenFlow connection
    openflow_connection ofp_connection;
    openflow_create_connection(&ofp_connection);
    while (looping) {
        openflow_control(&ofp_connection);
        // Query flows
        openflow_flows flows = {0};
        openflow_get_flows(&ofp_connection, &flows);
        // Get migrations
        openflow_get_flows(&ofp_connection, &flows);
        struct Migrations migrations = {0};
        get_migrations(&flows, map, &migrations);
        // Apply migrations
        for (int i = 0; i < migrations.nb_migrations; i++){
            apply_migration(&ofp_connection, &migrations.migrations[i]);
        }
        // Free flows
        openflow_free_flows(&flows);
        // Cleanup connections that haven't been filled
        hashmap_cleanup_inactive_flows(map);
        sleep(1);
    }
    // closing the listening socket
    openflow_terminate_connection(&ofp_connection);
    hashmap_destroy(map);
    return 0;
}
