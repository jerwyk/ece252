#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define BUF_SIZE 1048576  /* 1024*1024 = 1M */
#define BUF_INC  524288   /* 1024*512  = 0.5M */

#define round_increase(a, max) a = (a + 1) % max

/* a queue that can hold integers */
/* Note this structure can be used by shared memory,
   since the items field points to the memory right after it.
   Hence the structure and the data items it holds are in one
   continuous chunk of memory.

   The memory layout:
   +===============+
   | size          | 4 bytes
   +---------------+
   | pos           | 4 bytes
   +---------------+
   | items         | 8 bytes
   +---------------+
   | items[0]      | 4 bytes
   +---------------+
   | items[1]      | 4 bytes
   +---------------+
   | ...           | 4 bytes
   +---------------+
   | items[size-1] | 4 bytes
   +===============+
*/

/**
 * @brief calculate the total memory that the struct int_queue needs and
 *        the items[size] needs.
 * @param int size maximum number of integers the queue can hold
 * @return return the sum of buffer_queue_t size and the size of the data that
 *         items points to.
 */

int sizeof_shm_queue(int size)
{
    return (sizeof(buffer_queue_t) + sizeof(buffer_item_t) * size);
}

/**
 * @brief initialize the buffer_queue_t member fields.
 * @param buffer_queue_t *p points to the starting addr. of an buffer_queue_t struct
 * @param int queue_size max. number of items the queue can hold
 * @return 0 on success; non-zero on failure
 * NOTE:
 * The caller first calls sizeof_shm_queue() to allocate enough memory;
 * then calls the init_shm_queue to initialize the struct
 */
int init_shm_queue(buffer_queue_t *p, int queue_size)
{
    if ( p == NULL || queue_size == 0 ) {
        return 1;
    }

    p->size = queue_size;
    p->head  = 0;
    p->tail = 0;
    p->prod_index = 0;
    p->counter = 50;
    p->items = (buffer_item_t *) (p + sizeof(buffer_queue_t));
    return 0;
}

/**
 * @brief create a queue to hold size number of integers and its associated
 *      buffer_queue_t data structure. Put everything in one continous chunk of memory.
 * @param int size maximum number of integers the queue can hold
 * @return NULL if size is 0 or malloc fails
 */

buffer_queue_t *create_queue(int size)
{
    int mem_size = 0;
    buffer_queue_t *pqueue = NULL;
    
    if ( size == 0 ) {
        return NULL;
    }

    mem_size = sizeof_shm_queue(size);
    pqueue = malloc(mem_size);

    if ( pqueue == NULL ) {
        perror("malloc");
    } else {
        char *p = (char *)pqueue;
        pqueue->items = (buffer_item_t *) (p + sizeof(buffer_queue_t));
        pqueue->size = size;
        pqueue->head = 0;
        pqueue->tail = 0;
    }

    return pqueue;
}

/**
 * @brief release the memory
 * @param buffer_queue_t *p the address of the buffer_queue_t data structure
 */

void destroy_queue(buffer_queue_t *p)
{
    if ( p != NULL ) {
        free(p);
    }
}

/**
 * @brief check if the queue is full
 * @param buffer_queue_t *p the address of the buffer_queue_t data structure
 * @return non-zero if the queue is full; zero otherwise
 */

int is_full(buffer_queue_t *p)
{
    if ( p == NULL ) {
        return 0;
    }
    return ((p->tail + 1) % p->size == (p->head));
}

/**
 * @brief check if the queue is empty 
 * @param buffer_queue_t *p the address of the buffer_queue_t data structure
 * @return non-zero if the queue is empty; zero otherwise
 */

int is_empty(buffer_queue_t *p)
{
    if ( p == NULL ) {
        return 0;
    }
    return ( p->head == p->tail );
}

/**
 * @brief push one integer onto the queue 
 * @param buffer_queue_t *p the address of the buffer_queue_t data structure
 * @param int item the integer to be pushed onto the queue 
 * @return 0 on success; non-zero otherwise
 */

int enqueue(buffer_queue_t *p, buffer_item_t *item)
{
    if ( p == NULL ) {
        return -1;
    }

    if ( !is_full(p) ) {
        round_increase(p->tail, p->size);
        memcpy(p->items + p->tail, item, sizeof(buffer_item_t));
        return 0;
    } else {
        return -1;
    }
}

/**
 * @brief push one integer onto the queue 
 * @param buffer_queue_t *p the address of the buffer_queue_t data structure
 * @param int *item output parameter to save the integer value 
 *        that pops off the queue 
 * @return 0 on success; non-zero otherwise
 */

int dequeue(buffer_queue_t *p, buffer_item_t *p_item)
{
    if ( p == NULL ) {
        return -1;
    }

    if ( !is_empty(p) ) {
        *p_item = p->items[p->head];
        round_increase(p->head, p->size);
        return 0;
    } else {
        return 1;
    }
}
