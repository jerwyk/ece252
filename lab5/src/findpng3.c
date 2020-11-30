#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/multi.h>
#include <search.h>
#include <sys/time.h>
#include "util.h"
#include "crawler.h"
#include "png.h"

#define ECE252_HEADER "X-Ece252-Fragment: "
#define URL_BUF_SIZE 2048
#define MAX_WAIT_MSECS 30 * 1000

char* url_check_1 = "http://";
char* url_check_2 = "https://";
char url_buf[URL_BUF_SIZE][256];
int url_buf_tail = 0;
struct hsearch_data visited_urls;
struct hsearch_data visited_pngs;
struct url_queue_t url_frontier;

int image_num;
int connection_num;
FILE *f_log = NULL;
FILE *f_result = NULL;

int finished = 0;

int main(int argc, char **argv)
{
	/* used for program timing */
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
	
    image_num = 50;
    connection_num = 1;
    char *seed_url = NULL;
	
    /* parse options */
    if(argc > 1)
    {
        for(int i = 1; i < argc - 1; ++i)
        {
            if(strcmp(argv[i], "-t") == 0)
            {
                connection_num = strtol(argv[++i], NULL, 10);
                if(connection_num < 1)
                {
                    printf("You must have at least 1 connection.\n");
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
			printf("Usage: ./findpng3 -t T -m M seed_url\n");
			printf("Example: ./findpng3 -t 10 -m 50 http://ece252-1.uwaterloo.ca/lab4\n");
			return -4;
	}

    /* initializations */
    f_result = fopen("png_urls.txt", "w");
    url_entry_t *seed = (url_entry_t *)malloc(sizeof(url_entry_t));
    strcpy(seed->url, seed_url);
    hcreate_r(URL_BUF_SIZE, &visited_urls);
    hcreate_r(image_num, &visited_pngs);

    STAILQ_INIT(&url_frontier);
	
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURLM* multi_handle = curl_multi_init();
    CURLMsg *msg = NULL;
    CURL *eh = NULL;
    int msgs_left = 0;
    int handle_working = 0;
    int handle_finished = 0;

    CURL* easy_handle = easy_handle_init(seed_url);
    curl_multi_add_handle(multi_handle, easy_handle);

	for(int i = 0; i < connection_num - 1; ++i){
		CURL* easy_handle = easy_handle_init("");
		curl_multi_add_handle(multi_handle, easy_handle);
		/* remember to curl_easy_cleanup when done */
	}

    ENTRY new_url;
    ENTRY *res;
    new_url.data = NULL;
    new_url.key = seed_url;
    strcpy(url_buf[url_buf_tail], seed_url);
    new_url.key = url_buf[url_buf_tail++];
    hsearch_r(new_url, ENTER, &res, &visited_urls);

    curl_multi_perform(multi_handle, &handle_working);

    while(!finished)
    {
        do {
            int numfds=0;
            int res = curl_multi_wait(multi_handle, NULL, 0, MAX_WAIT_MSECS, &numfds);
            if(res != CURLM_OK) {
                fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
                return EXIT_FAILURE;
            }
            curl_multi_perform(multi_handle, &handle_working);

        } while(handle_working);

        handle_finished = 0;

        while ( ( msg = curl_multi_info_read( multi_handle, &msgs_left ) ) ) 
        {
            if ( msg->msg == CURLMSG_DONE ) {
                eh = msg->easy_handle;

                RECV_BUF *buf;
                curl_easy_getinfo(eh, CURLINFO_PRIVATE, &buf);

                process_data(eh, buf);
                recv_buf_cleanup(buf);
                free(buf);

                if(!STAILQ_EMPTY(&url_frontier))
                {
                    curl_multi_remove_handle( multi_handle, eh );
                    url_entry_t *url_entry = STAILQ_FIRST(&url_frontier);
                    STAILQ_REMOVE_HEAD(&url_frontier, pointers);    
                    eh = easy_handle_init(url_entry->url);
                    free(url_entry);
                    curl_multi_add_handle(multi_handle, eh);
                }
                else
                {
                    handle_finished++;
                    if(handle_finished == connection_num)
                    {
                        finished = 1;
                        curl_easy_cleanup(eh);
                    }
                    else
                    {
                        curl_multi_remove_handle( multi_handle, eh );
                        curl_easy_setopt(eh, CURLOPT_URL, "");
                        curl_multi_add_handle(multi_handle, eh);
                    }            
                }         

            } else {
                fprintf( stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg );
            }
        }
    }


    /* clean up */
	if(f_log != NULL){
		fclose(f_log);
	}
    fclose(f_result);
    xmlCleanupParser();
	hdestroy_r(&visited_urls);
    hdestroy_r(&visited_pngs);
	curl_multi_cleanup(multi_handle);
	curl_global_cleanup();
	
	free(seed);
	
	while(!STAILQ_EMPTY(&url_frontier))
    {
        url_entry_t *url_entry = STAILQ_FIRST(&url_frontier);
		STAILQ_REMOVE_HEAD(&url_frontier, pointers);
        free(url_entry);
	}

    gettimeofday(&t2, NULL);
    double t1_sec, t2_sec;
    t1_sec = t1.tv_sec + t1.tv_usec / 1000000.0;
    t2_sec = t2.tv_sec + t2.tv_usec / 1000000.0;

    printf("findpng3 execution time: %f seconds\n", t2_sec - t1_sec);
	
	return 0;
}