#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>
#include <search.h>
#include <semaphore.h>
#include <sys/time.h>
#include "util.h"
#include "crawler.h"
#include "png.h"

#define ECE252_HEADER "X-Ece252-Fragment: "
#define URL_BUF_SIZE 2048

char* url_check_1 = "http://";
char* url_check_2 = "https://";
char url_buf[URL_BUF_SIZE][256];
int url_buf_tail = 0;
struct hsearch_data visited_urls;
struct hsearch_data visited_pngs;
struct url_queue_t url_frontier;

int image_num;
int thread_working = 0;
int thread_num;
FILE *f_log = NULL;
FILE *f_result = NULL;

pthread_mutex_t mutex;
sem_t sem_frontier;
int finished = 0;

int main(int argc, char **argv)
{
	/* used for program timing */
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
	
    image_num = 50;
    thread_num = 1;
    char *seed_url = NULL;
	
    /* parse options */
    if(argc > 1)
    {
        for(int i = 1; i < argc - 1; ++i)
        {
            if(strcmp(argv[i], "-t") == 0)
            {
                thread_num = strtol(argv[++i], NULL, 10);
                if(thread_num < 1)
                {
                    printf("You must have at least 1 thread.\n");
                    return -1;
                }
            }
            else if(strcmp(argv[i], "-m") == 0)
            {
                image_num = strtol(argv[++i], NULL, 10);
                if(image_num < 1)
                {
                    printf("Number of PNGs need to be positive.\n");
                    return -2;
                }
            }
            else if(strcmp(argv[i], "-v") == 0)
            {
                f_log = fopen(argv[++i], "w");
            }
        }
    }
	
	/* checking if input url is valid */
    seed_url = argv[argc - 1];
	if(strstr(seed_url, url_check_1) == NULL && strstr(seed_url, url_check_2) == NULL){
			printf("Usage: ./findpng2 -t T -m M seed_url\n");
			printf("Example: ./findpng2 -t 10 -m 50 http://ece252-1.uwaterloo.ca/lab4\n");
			return -4;
	}

    /* initializations */
    f_result = fopen("png_urls.txt", "w");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    url_entry_t *seed = (url_entry_t *)malloc(sizeof(url_entry_t));
    strcpy(seed->url, seed_url);
    hcreate_r(URL_BUF_SIZE, &visited_urls);
    hcreate_r(image_num, &visited_pngs);

    STAILQ_INIT(&url_frontier);
    STAILQ_INSERT_TAIL(&url_frontier, seed, pointers);
	
	pthread_mutex_init(&mutex, NULL);
	sem_init(&sem_frontier, 0, 1);
	
    /* create threads to crawl the web */
	pthread_t* threads = malloc(sizeof(pthread_t) * thread_num);
	int* thread_status = malloc(sizeof(int) * thread_num);

    for(int i = 0; i < thread_num; ++i)
    {
		// run t_crawler with each thread with argument max
		thread_status[i] = pthread_create(&threads[i], NULL, t_crawler, NULL);
    }
	
	for(int i = 0; i < thread_num; ++i){
		if(thread_status[i] == 0){
			pthread_join(threads[i], NULL);
		}
	}

    /* clean up */
	if(f_log != NULL){
		fclose(f_log);
	}
    fclose(f_result);
    curl_global_cleanup();
    xmlCleanupParser();
	hdestroy_r(&visited_urls);
    hdestroy_r(&visited_pngs);
    free(thread_status);
    free(threads);
	
	while(!STAILQ_EMPTY(&url_frontier))
    {
        url_entry_t *url_entry = STAILQ_FIRST(&url_frontier);
		STAILQ_REMOVE_HEAD(&url_frontier, pointers);
        free(url_entry);
	}
	
	pthread_mutex_destroy(&mutex);
	sem_destroy(&sem_frontier);

    gettimeofday(&t2, NULL);
    double t1_sec, t2_sec;
    t1_sec = t1.tv_sec + t1.tv_usec / 1000000.0;
    t2_sec = t2.tv_sec + t2.tv_usec / 1000000.0;

    printf("findpng2 execution time: %f seconds\n", t2_sec - t1_sec);
	
	return 0;
}