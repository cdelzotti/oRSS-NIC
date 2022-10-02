#include "balancer.h"

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
        repartition->core_load[i].load = 0;
        repartition->core_load[i].biggestLoad_idx = 0;
        __uint128_t biggestLoad = 0;
        for (int j = 0; j < repartition->core_load[i].nb_flows; j++){
            __uint128_t currentLoad = ringbuffer_get_last(repartition->core_load[i].flows[j]);
            if (currentLoad > biggestLoad){
                biggestLoad = currentLoad;
                repartition->core_load[i].biggestLoad_idx = j;
            }
            repartition->core_load[i].load += currentLoad;
        }
    }
}

void balancer_migrate(struct CoreLoad *bigCore, struct CoreLoad *smallCore){
    // Migrate the biggest flow
    int biggestLoad_idx = bigCore->biggestLoad_idx;
    struct RingBuffer *flow = bigCore->flows[biggestLoad_idx];
    struct FiveTuple key = bigCore->flowKeys[biggestLoad_idx];
    // Remove the flow from the big core
    bigCore->flows[biggestLoad_idx] = bigCore->flows[bigCore->nb_flows];
    bigCore->flowKeys[biggestLoad_idx] = bigCore->flowKeys[bigCore->nb_flows];
    bigCore->nb_flows--;
    // Add the flow to the small core
    smallCore->nb_flows++;
    smallCore->flows[smallCore->nb_flows] = flow;
    smallCore->flowKeys[smallCore->nb_flows] = key;
    // Update the assigned core
    flow->assigned_core = smallCore->core_idx;
    // Update the load of the cores
    if (smallCore->flows[smallCore->biggestLoad_idx] < bigCore->flows[biggestLoad_idx]){
        smallCore->biggestLoad_idx = smallCore->nb_flows;
    }
}

void balancer_balance(struct HashMap *hashmap, int nbCores){
    // Compute the repartition
    struct Repartition repartition;
    balancer_compute_repartition(&repartition, hashmap, nbCores);
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
    if (largest_imbalance > 1 + IMBALANCE_THRESHOLD || smallest_imbalance < 1 - IMBALANCE_THRESHOLD) {
        // There are too much flows on the core with the biggest load
    }
}


void balancer_free(struct Repartition *repartition){
    free(repartition->core_load);
}