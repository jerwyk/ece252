#include "crawler.h"
#include "util.h"
#include <search.h>

extern hashmap_t visited_urls;
extern hashmap_t visited_pngs;
extern struct url_queue_t url_frontier;

extern pthread_rwlock_t rw_urls;
extern pthread_rwlock_t rw_pngs;
extern pthread_mutex_t mutex;
extern pthread_mutex_t file_mutex;
extern semt_t sem_frontier;
extern volatile int num_pngs;

void* t_crawler(void* param)
{

    int max = (int)*param;



}