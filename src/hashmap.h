#include "ringbuffer.h"
#include "bpf/xdp.bpf.h"
#include "env.h"

/*
Represents a matching key-value pair in the hashmap.
*/
struct key_value_pair {
    uint8_t valid;
    struct FiveTuple key;
    struct RingBuffer *value;
};

struct HashMap {
    struct key_value_pair *map;
    int size;
};

/*
Returns a pointer to a new hashmap.
This pointer will need to be freed with hashmap_destroy.
*/
struct HashMap *hashmap_init();

/* Free the memory allocated to the hashmap and its ringbuffers 
    Parameters:
        hashmap: The hashmap to free
*/
void hashmap_destroy(struct HashMap *hashmap);

/* Insert a key-value pair into the hashmap
    Parameters:
        hashmap: The hashmap to insert into
        key: The key to insert
        value: The value-pointer to insert
*/
void hashmap_insert(struct HashMap *hashmap, struct FiveTuple key, struct RingBuffer *value);

/*
    Returns the value-pointer associated with the given key, or NULL if the key is not in the hashmap.
    Parameters:
        hashmap: The hashmap to search
        key: The key to search for
*/
struct RingBuffer *hashmap_get(struct HashMap *hashmap, struct FiveTuple *key);

/*
    Removes the key-value pair associated with the given key from the hashmap.
    Parameters:
        hashmap: The hashmap to remove from
        key: The key to remove
*/
void hashmap_remove(struct HashMap *hashmap, struct FiveTuple *key);

/*
    Returns a pointer to a new ringbuffer.
    This pointer will be freed either by `hashmap_destroy` or by `hashmap_remove`.
    Parameters:
        hashmap: The hashmap to insert into
        key: The key to insert
*/
struct RingBuffer *hashmap_new(struct HashMap *hashmap, struct FiveTuple *key);

/*
    For a given key, returns the following one in the hashmap. There is no guarantee on the order of the keys.
    By calling this function repeatedly, you can iterate over all the keys in the hashmap.
    Parameters:
        hashmap: The hashmap to iterate over
        current_key: The key to start from. If NULL, the first key in the hashmap will be returned.
        next_key: The key that follows the current key.
        next_value: The value-pointer associated with the next key. This will be NULL if the next key is not in the hashmap.
    Returns:
        0 if a next key couldn't.
*/
uint8_t hashmap_get_next(struct HashMap *hashmap, struct FiveTuple *current_key, struct FiveTuple *next_key, struct RingBuffer **next_value);