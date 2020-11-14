#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>
#include <search.h>
#include <sys/time.h>
#include "png.h"

#define ECE252_HEADER "X-Ece252-Fragment: "
#define STRIP_NUM 50
typedef struct hsearch_data hashmap_t;

hashmap_t visited_urls;
hashmap_t visited_pngs;
char *url_frontier;

int main(int argc, char **argv)
{
	/* used for program timing */
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
	
    int image_num = 50;
    int thread_num = 1;
    char *logfile = NULL;
    char *seed_url = NULL;
	void** retval;
	
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
                logfile = argv[++i];
            }
        }
        seed_url = argv[argc - 1];
    }

    /* initializations */
    curl_global_init(CURL_GLOBAL_DEFAULT);
	
    /* create threads to crawl the web */
	pthread_t* threads = malloc(sizeof(pthread_t) * thread_num);
	int* thread_status = malloc(sizeof(int) * thread_num);
	
    for(int i = 0; i < thread_num; ++i)
    {
		// run t_crawler with each thread with argument max
		thread_status[i] = pthread_create(&threads[i], NULL, t_crawler, max);
    }
	
	for(int i = 1; i < thread_num; ++i){
		if(thread_status[i] == 0){
			pthread_join(threads[i], retval);
		}
	}

    /* clean up */
    curl_global_cleanup();

    gettimeofday(&t2, NULL);
    double t1_sec, t2_sec;
    t1_sec = t1.tv_sec + t1.tv_usec / 1000000.0;
    t2_sec = t2.tv_sec + t2.tv_usec / 1000000.0;

    printf("findpng2 execution time: %f seconds\n", t2_sec - t1_sec);
	
	return 0;
}