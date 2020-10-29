#pragma once

struct buffer_queue {
    int head;
    int tail;
    int size;
    buffer_item_t *items;
};

struct buffer_item {
    int seg_num;
    int size;
    char buf[1048576];
};

typedef struct buffer_item buffer_item_t;
typedef struct buffer_queue buffer_queue_t;

int sizeof_shm_queue(int size);
int init_shm_queue(buffer_queue_t *p, int queue_size);
buffer_queue_t *create_queue(int size);
void destroy_queue(buffer_queue_t *p);
int is_full(buffer_queue_t *p);
int is_empty(buffer_queue_t *p);
int enqueue(buffer_queue_t *p, buffer_item_t *item);
int dequeue(buffer_queue_t *p, buffer_item_t *p_item);