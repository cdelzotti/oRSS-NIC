#include "balancer.h"


void balancer_print_migrations(struct Migrations *migrations, struct Repartition *repartition, int nbCores, struct HashMap *hashmap){
    // Describe migrations
    for (int i = 0; i < migrations->nb_migrations; i++){
        struct RingBuffer *ring_buffer = hashmap_get(hashmap, &migrations->migrations[i].key);
        printf("Migrate flow %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d (%lu) to core %d\n",
        migrations->migrations[i].key.src_ip & 0xFF,
        (migrations->migrations[i].key.src_ip >> 8) & 0xFF,
        (migrations->migrations[i].key.src_ip >> 16) & 0xFF,
        (migrations->migrations[i].key.src_ip >> 24) & 0xFF,
        migrations->migrations[i].key.src_port,
        migrations->migrations[i].key.dst_ip & 0xFF,
        (migrations->migrations[i].key.dst_ip >> 8) & 0xFF,
        (migrations->migrations[i].key.dst_ip >> 16) & 0xFF,
        (migrations->migrations[i].key.dst_ip >> 24) & 0xFF,
        migrations->migrations[i].key.dst_port,
        ringbuffer_get_last(ring_buffer),
        migrations->migrations[i].destination_core);
    }
    printf("--------------------\n");
    // Describe core load
    for (int i = 0; i < nbCores; i++)
        printf("Core %d: %u flows, %lu load\n", i, repartition->core_load[i].nb_flows, repartition->core_load[i].load);
    printf("\n");
    // Describe flows
    for (int i = 0; i < nbCores; i++){
        printf("Core %d:\n", i);
        for (int j = 0; j < repartition->core_load[i].nb_flows; j++){
            printf("[%u]%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d to core %d\n",
            repartition->core_load[i].flowKeys[j].proto,
            repartition->core_load[i].flowKeys[j].src_ip & 0xFF,
            (repartition->core_load[i].flowKeys[j].src_ip >> 8) & 0xFF,
            (repartition->core_load[i].flowKeys[j].src_ip >> 16) & 0xFF,
            (repartition->core_load[i].flowKeys[j].src_ip >> 24) & 0xFF,
            repartition->core_load[i].flowKeys[j].src_port,
            repartition->core_load[i].flowKeys[j].dst_ip & 0xFF,
            (repartition->core_load[i].flowKeys[j].dst_ip >> 8) & 0xFF,
            (repartition->core_load[i].flowKeys[j].dst_ip >> 16) & 0xFF,
            (repartition->core_load[i].flowKeys[j].dst_ip >> 24) & 0xFF,
            repartition->core_load[i].flowKeys[j].dst_port,
            repartition->core_load[i].core_idx);
        }
        printf("\n");
    }
}


void balancer_update_core_info(struct CoreLoad *core){
    core->load = 0;
    core->biggestLoad_idx = 0;
    uint64_t biggestLoad = 0;
    for (int j = 0; j < core->nb_flows; j++){
        uint64_t currentLoad = ringbuffer_get_last(core->flows[j]);
        if (currentLoad > biggestLoad){
            biggestLoad = currentLoad;
            core->biggestLoad_idx = j;
        }
        core->load += currentLoad;
    }
}

void balancer_compute_repartition(struct Repartition *repartition, struct HashMap *hashmap, uint8_t nbCores){
    // Initialize the repartition
    repartition->core_load = calloc(nbCores, sizeof(struct CoreLoad));
    // Iterate over the hashmap
    struct FiveTuple previous_key;
    struct FiveTuple current_key;
    struct RingBuffer *value;
    while (hashmap_get_next(hashmap, &previous_key, &current_key, &value)){
        // Memorized assigned flows
        repartition->core_load[value->assigned_core].flowKeys[repartition->core_load[value->assigned_core].nb_flows] = current_key;
        repartition->core_load[value->assigned_core].flows[repartition->core_load[value->assigned_core].nb_flows] = value;
        repartition->core_load[value->assigned_core].nb_flows++;

        previous_key = current_key;
    }
    // Compute the load of each core
    for (int i = 0; i < nbCores; i++){
        repartition->core_load[i].core_idx = i;
        balancer_update_core_info(&repartition->core_load[i]);
    }
}

struct Migration balancer_migrate(struct CoreLoad *bigCore, struct CoreLoad *smallCore){
    // Migrate the biggest flow
    int biggestLoad_idx = bigCore->biggestLoad_idx;
    struct RingBuffer *flow = bigCore->flows[biggestLoad_idx];
    struct FiveTuple key = bigCore->flowKeys[biggestLoad_idx];
    // Remove the flow from the big core
    bigCore->nb_flows--;
    bigCore->flows[biggestLoad_idx] = bigCore->flows[bigCore->nb_flows];
    bigCore->flowKeys[biggestLoad_idx] = bigCore->flowKeys[bigCore->nb_flows];
    // Add the flow to the small core
    smallCore->flows[smallCore->nb_flows] = flow;
    smallCore->flowKeys[smallCore->nb_flows] = key;
    smallCore->nb_flows++;
    // Update the assigned core
    flow->assigned_core = smallCore->core_idx;
    // Update the load of the cores
    balancer_update_core_info(bigCore);
    balancer_update_core_info(smallCore);
    // Return a description of the migration
    struct Migration migration;
    migration.key = key;
    migration.destination_core = smallCore->core_idx;
    return migration;
}

void balancer_free(struct Repartition *repartition){
    free(repartition->core_load);
}

void balancer_balance(struct HashMap *hashmap, int nbCores, struct Migrations *migrations){
    // Compute the repartition
    struct Repartition repartition;
    balancer_compute_repartition(&repartition, hashmap, nbCores);
    while (migrations->nb_migrations < MAX_REBALANCE_ITERATIONS)
    {
        // Compute the average load and find the core with the biggest load
        // & the core with the smallest load
        uint64_t averageLoad = 0;
        uint64_t biggestLoad = 0;
        uint64_t smallestLoad = UINT64_MAX;
        int biggestLoad_idx = 0;
        int smallestLoad_idx = 0;
        for (int i = 0; i < nbCores; i++){
            averageLoad += repartition.core_load[i].load;
            if (repartition.core_load[i].load > biggestLoad){
                biggestLoad = repartition.core_load[i].load;
                biggestLoad_idx = i;
            }
            if (repartition.core_load[i].load < smallestLoad){
                smallestLoad = repartition.core_load[i].load;
                smallestLoad_idx = i;
            }
        }
        averageLoad /= nbCores;
        // Check if the load is balanced enough
        long double largest_imbalance = (long double) biggestLoad / (long double) averageLoad;
        long double smallest_imbalance = (long double) smallestLoad / (long double) averageLoad;
        if (largest_imbalance > 1 + IMBALANCE_THRESHOLD || smallest_imbalance < 1 - IMBALANCE_THRESHOLD && averageLoad > 0) {
            // printf("Rebalancing: largest_imbalance = %Lf, smallest_imbalance = %Lf, average_load = %lu\n", largest_imbalance, smallest_imbalance, averageLoad);
            // There are too much flows on the core with the biggest load
            migrations->migrations[migrations->nb_migrations] = balancer_migrate(&repartition.core_load[biggestLoad_idx], &repartition.core_load[smallestLoad_idx]);
            migrations->nb_migrations++;
        } else {
            // The load is balanced enough
            break;
        }
    }
    // balancer_print_migrations(migrations, &repartition, nbCores, hashmap);
    balancer_free(&repartition);
}
