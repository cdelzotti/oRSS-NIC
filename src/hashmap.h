#include "ringbuffer.h"
#include "bpf/xdp.bpf.h"
#include "env.h"

struct key_value_pair {
    uint8_t valid;
    struct FiveTuple key;
    struct RingBuffer *value;
};

struct HashMap {
    struct key_value_pair *map;
    int size;
};

struct HashMap *hashmap_init();

void hashmap_destroy(struct HashMap *hashmap);

void hashmap_insert(struct HashMap *hashmap, struct FiveTuple key, struct RingBuffer *value);

struct RingBuffer *hashmap_get(struct HashMap *hashmap, struct FiveTuple *key);

void hashmap_remove(struct HashMap *hashmap, struct FiveTuple *key);

struct RingBuffer *hashmap_new(struct HashMap *hashmap, struct FiveTuple *key);