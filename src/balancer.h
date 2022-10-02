#include "env.h"
#include "ringbuffer.h"
#include "hashmap.h"
#include "src/bpf/xdp.bpf.h"
#include <math.h>

struct CoreLoad {
    uint8_t core_idx;
    __uint128_t load;
    int biggestLoad_idx;
    int nb_flows;
    struct RingBuffer* flows[HASHMAP_SIZE];
    struct FiveTuple flowKeys[HASHMAP_SIZE];
};

struct Repartition {
    struct CoreLoad *core_load;
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
void balancer_balance(struct HashMap *flows, int nbCores);

/*
    Free the memory allocated for the repartition
*/
void balancer_free(struct Repartition *repartition);