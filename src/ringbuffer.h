#include "env.h"
#include "bits/stdint-uintn.h"

struct ringbuffer {
    int pos;
    int size;
    uint64_t buffer[RING_SIZE];
};

/* Initialize ringbuffer 
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
void ringbuffer_init(struct ringbuffer *rb);

/* Free ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
void ringbuffer_destroy(struct ringbuffer *rb);

/* Add element to ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
    uint64_t element: element to add
*/
void ringbuffer_add(struct ringbuffer *rb, uint64_t value);

/* Get last element from ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
*/
uint64_t ringbuffer_get_last(struct ringbuffer *rb);

/* Get average value of ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
    int count: number of elements to include in the average
*/
uint64_t ringbuffer_get_average(struct ringbuffer *rb, int count);

/* Get an estimate of the next value in the ringbuffer
Parameters:
    ringbuffer* rb: pointer to ringbuffer
    int count : number of last elements to use for the estimation
*/
uint64_t ringbuffer_predict_next(struct ringbuffer *rb, int count);
