#include "hashmap.h"

struct HashMap *hashmap_init() {
    struct HashMap *hashmap = calloc(1, sizeof(struct HashMap));
    hashmap->map = calloc(HASHMAP_SIZE, sizeof(struct key_value_pair));
    hashmap->size = 0;
    return hashmap;
}

void hashmap_destroy(struct HashMap *hashmap) {
    for (int i = 0; i < hashmap->size; i++) {
        ringbuffer_destroy(hashmap->map[i].value);
    }
    free(hashmap->map);
    free(hashmap);
}

uint8_t five_tuple_equals(struct FiveTuple *a, struct FiveTuple *b) {
    return a->src_ip == b->src_ip && a->dst_ip == b->dst_ip && a->src_port == b->src_port && a->dst_port == b->dst_port && a->proto == b->proto;
}

int hashmap_contains(struct HashMap *hashmap, struct FiveTuple *key) {
    for (int i = 0; i < hashmap->size; i++) {
        if (five_tuple_equals(&hashmap->map[i].key, key) && hashmap->map[i].valid) {
            return i;
        }
    }
    return -1;
}

void hashmap_insert_at_index(struct HashMap *hashmap, int index, struct RingBuffer *value, struct FiveTuple key) {
    hashmap->map[index].key = key;
    hashmap->map[index].value = value;
    hashmap->map[index].valid = 1;
}

int hashmap_first_free_slot(struct HashMap *hashmap) {
    for (int i = 0; i < HASHMAP_SIZE; i++) {
        if (!hashmap->map[i].valid) {
            return i;
        }
    }
    return -1;
}

void hashmap_insert(struct HashMap *hashmap, struct FiveTuple key, struct RingBuffer *value) {
    int index = hashmap_contains(hashmap, &key);
    // If the key is already in the hashmap, update the value
    if (index >= 0){
        hashmap_insert_at_index(hashmap, index, value, key);
        return;
    }
    // If the key is not in the hashmap, insert it at the first free slot
    index = hashmap_first_free_slot(hashmap);
    if (index >= 0){
        hashmap_insert_at_index(hashmap, index, value, key);
        hashmap->size++;
        return;
    }
    // Raise an error if the hashmap is full
    printf("Hashmap is full!");
    exit(1);
}

struct RingBuffer *hashmap_get(struct HashMap *hashmap, struct FiveTuple *key) {
    int index = hashmap_contains(hashmap, key);
    if (index >= 0){
        return hashmap->map[index].value;
    }
    return (void *)0;
}

void hashmap_remove(struct HashMap *hashmap, struct FiveTuple *key) {
    int index = hashmap_contains(hashmap, key);
    if (index >= 0){
        hashmap->map[index].valid = 0;
        ringbuffer_destroy(hashmap->map[index].value);
        hashmap->size--;
    }
}

struct RingBuffer *hashmap_new(struct HashMap *hashmap, struct FiveTuple *key) {
    struct RingBuffer *value = ringbuffer_init();
    hashmap_insert(hashmap, *key, value);
    return value;
}

uint8_t hashmap_get_next(struct HashMap *hashmap, struct FiveTuple *current_key, struct FiveTuple *next_key, struct RingBuffer **next_value) {
    int index = hashmap_contains(hashmap, current_key);
    for (int i = index + 1; i < HASHMAP_SIZE; i++) {
        if (hashmap->map[i].valid) {
            *next_key = hashmap->map[i].key;
            *next_value = hashmap->map[i].value;
            return 1;
    }
    *next_key = (struct FiveTuple){0};
    *next_value = (void *)0;
    return 0;
    }
}