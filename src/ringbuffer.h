#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "env.h"
#include "bits/stdint-uintn.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

struct RingBuffer {
    uint8_t assigned_core; // Each ringbuffer corresponds to a flow, and each flow is assigned to a core
    int pos; // The position of the current element
    int size; // The number of elements in the buffer
    uint64_t buffer[RING_SIZE];
    uint8_t is_active;
};

/* Initialize ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
struct RingBuffer * ringbuffer_init();

/* Free ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
void ringbuffer_destroy(struct RingBuffer *rb);

/* Add element to ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
    uint64_t element: element to add
*/
void ringbuffer_add(struct RingBuffer *rb, uint64_t value);

/* Get last element from ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
uint64_t ringbuffer_get_last(struct RingBuffer *rb);

/* Get average value of ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
    int count: number of elements to include in the average
*/
uint64_t ringbuffer_get_average(struct RingBuffer *rb, int count);

/* Get an estimate of the next value in the ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
    int count : number of last elements to use for the estimation
*/
uint64_t ringbuffer_predict_next(struct RingBuffer *rb, int count);

/* Returns true if the ringbuffer hasn't been updated for `CONN_TIMEOUT` seconds
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
uint8_t ringbuffer_is_timedout(struct RingBuffer *rb);

#endif