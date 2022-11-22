#ifndef BALANCER_H
#define BALANCER_H

#include "env.h"
#include "hashmap.h"
#include <math.h>
#include <stdlib.h>


struct CoreLoad {
    uint8_t core_idx;
    uint64_t load;
    int biggestLoad_idx;
    int nb_flows;
    struct RingBuffer* flows[HASHMAP_SIZE];
    struct FiveTuple flowKeys[HASHMAP_SIZE];
};

struct Repartition {
    struct CoreLoad *core_load;
};

struct Migration {
    struct FiveTuple key;
    int destination_core;
};

struct Migrations {
    int nb_migrations;
    struct Migration migrations[MAX_REBALANCE_ITERATIONS];
};

/*
    Compute the load of each core and return a representation of the load within the struct Repartition
    Parameters:
        repartition: initialized struct to store the load of each core
        hashmap: hashmap containing the flows
        nbCores: number of cores
*/
void balancer_compute_repartition(struct Repartition *repartition, struct HashMap *hashmap, uint8_t nbCores);

/*
    Balance the flows between the cores
    Parameters:
        hashmap: hashmap containing the flows
*/
void balancer_balance(struct HashMap *hashmap, int nbCores, struct Migrations *migrations);

/*
    Free the memory allocated for the repartition
*/
void balancer_free(struct Repartition *repartition);

#endif
