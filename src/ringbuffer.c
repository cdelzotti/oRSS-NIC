#include "ringbuffer.h"

struct RingBuffer *ringbuffer_init(){
    struct RingBuffer* new_buf = calloc(1, sizeof(struct RingBuffer));
    // new_buf->assigned_core = rand() % NB_CORES; // Assign a random core to the flow
    new_buf->assigned_core = 0;
    return new_buf;
}

void ringbuffer_destroy(struct RingBuffer *rb){
    free(rb);
}

void ringbuffer_add(struct RingBuffer *rb, uint64_t value){
    rb->pos = (rb->pos + 1) % RING_SIZE;
    rb->buffer[rb->pos] = value;
    if (rb->size < RING_SIZE){
        rb->size++;
    }
}

uint64_t ringbuffer_get_last(struct RingBuffer *rb){
    if (rb->size == 0){
        return 0;
    }
    return rb->buffer[rb->pos];
}

uint64_t ringbuffer_get_average(struct RingBuffer *rb, int count) {
    if (rb->size == 0){
        return 0;
    }
    int i = 0;
    uint64_t sum = 0;
    int pos = rb->pos;
    while (i < count && i < rb->size){
        sum += rb->buffer[pos];
        pos = (pos - 1) % RING_SIZE;
        if (pos < 0){
            pos = RING_SIZE - 1;
        }
        i++;
    }
    return sum / count;
}

uint64_t ringbuffer_predict_next(struct RingBuffer *rb, int count){
    if (rb->size == 0){
        return 0;
    }
    uint64_t last = ringbuffer_get_last(rb);
    uint64_t average = ringbuffer_get_average(rb, count);
    return last + (average - last);
}

uint8_t ringbuffer_is_flat(struct RingBuffer *rb){
    if (rb->size < RING_SIZE){
        return 0;
    }
    uint64_t last = ringbuffer_get_last(rb);
    int i = 0;
    int pos = rb->pos;
    while (i < rb->size){
        if (rb->buffer[pos] != last){
            return 0;
        }
        pos = (pos - 1) % RING_SIZE;
        if (pos < 0){
            pos = RING_SIZE - 1;
        }
        i++;
    }
    return 1;
}