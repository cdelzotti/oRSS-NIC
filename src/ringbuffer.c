#include "ringbuffer.h"

void ringbuffer_init(struct ringbuffer *rb){
    rb = calloc(sizeof(struct ringbuffer));
}

void ringbuffer_destroy(struct ringbuffer *rb){
    free(rb);
}

void ringbuffer_add(struct ringbuffer *rb, uint64_t value){
    if (rb->size == 0){
        rb->buffer[0] = value;
        return;
    }
    rb->pos = (rb->pos + 1) % RING_SIZE;
    rb->buffer[rb->pos] = value;
    if (rb->size < RING_SIZE){
        rb->size++;
    }
}

uint64_t ringbuffer_get_last(struct ringbuffer *rb){
    if (rb->size == 0){
        return 0;
    }
    return rb->buffer[rb->pos];
}

uint64_t ringbuffer_get_average(struct ringbuffer *rb, int count) {
    if (rb->size == 0){
        return 0;
    }
    int i = 0;
    uint64_t sum = 0;
    int pos = rb->pos;
    while (i < count && i < rb->size){
        sum += rb->buffer[pos];
        pos = (pos - 1) % RING_SIZE;
        i++;
    }
    return sum / count;
}

uint64_t ringbuffer_predict_next(struct ringbuffer *rb, int count){
    if (rb->size == 0){
        return 0;
    }
    uint64_t last = ringbuffer_get_last(rb);
    uint64_t average = ringbuffer_get_average(rb, count);
    return last + (average - last);
}