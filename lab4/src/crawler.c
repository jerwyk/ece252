#include "crawler.h"
#include "util.h"
#include <search.h>
#include <pthread.h>

extern int image_num;
extern int thread_num;
extern char *logfile;
extern char *seed_url;

extern hashmap_t visited_urls;
extern hashmap_t visited_pngs;
extern struct url_queue_t url_frontier;

extern pthread_rwlock_t rw_urls;
extern pthread_rwlock_t rw_pngs;
extern pthread_mutex_t mutex;
extern pthread_mutex_t file_mutex;
extern sem_t sem_frontier;
extern pthread_cond_t cond_frontier;
extern volatile int num_pngs;
extern int num_thread_wait;

extern int finished;

int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf);

void* t_crawler(void* param)
{
    char * url;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        {
            num_thread_wait++;
            if(STAILQ_EMPTY(&url_frontier))
            {
                if(num_thread_wait < thread_num)
                {
                    pthread_cond_wait(&cond_frontier, &mutex);
                }
                else
                {
                    finished = 1;
                    pthread_cond_broadcast(&cond_frontier);
                } 
            }
            if(finished)
            {
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }
            num_thread_wait--;
            url = STAILQ_FIRST(&url_frontier);
            STAILQ_REMOVE_HEAD(&url_frontier, pointers);
        }
        pthread_mutex_unlock(&mutex);

        CURL *curl_handle;
        CURLcode res;
        curl_handle = easy_handle_init(&recv_buf, url);

        RECV_BUF recv_buf;
        res = curl_easy_perform(curl_handle);

        /* process the download data */
        process_data(curl_handle, &recv_buf);
    }
}

int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    char fname[256];
    int follow_relative_link = 1;
    char *url = NULL; 
    pid_t pid =getpid();

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    find_http(p_recv_buf->buf, p_recv_buf->size, follow_relative_link, url); 
    return 0;
}

int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    pid_t pid =getpid();
    char fname[256];
    char *eurl = NULL;          /* effective URL */
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &eurl);
    if ( eurl != NULL) {
        printf("The PNG url is: %s\n", eurl);
    }

    sprintf(fname, "./output_%d_%d.png", p_recv_buf->seq, pid);
    return write_file(fname, p_recv_buf->buf, p_recv_buf->size);
}
/**
 * @brief process teh download data by curl
 * @param CURL *curl_handle is the curl handler
 * @param RECV_BUF p_recv_buf contains the received data. 
 * @return 0 on success; non-zero otherwise
 */

int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    CURLcode res;
    char fname[256];
    pid_t pid =getpid();
    long response_code;

    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

    if ( response_code >= 400 ) { 
    	fprintf(stderr, "Error.\n");
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	;
    } else {
        fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {
        return process_html(curl_handle, p_recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        return process_png(curl_handle, p_recv_buf);
    } else {
        sprintf(fname, "./output_%d", pid);
    }

    return write_file(fname, p_recv_buf->buf, p_recv_buf->size);
}