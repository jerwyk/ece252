#define _GNU_SOURCE

#include "crawler.h"
#include "util.h"
#include "png.h"
#include <search.h>
#include <pthread.h>
#include <semaphore.h>

#define CT_PNG  "image/png"
#define CT_HTML "text/html"

typedef struct hsearch_data hashmap_t;

extern int image_num;
extern int thread_num;
extern int thread_working;
extern FILE *f_log;
extern FILE *f_result;

extern char url_buf[2048][256];
extern int url_buf_tail;
extern struct url_queue_t url_frontier;
extern struct hsearch_data visited_urls;
extern struct hsearch_data visited_pngs;

extern pthread_mutex_t mutex;
extern sem_t sem_frontier;

extern int finished;

int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf);

void* t_crawler(void* param)
{
    CURL *curl_handle;
    RECV_BUF recv_buf;
    curl_handle = easy_handle_init(&recv_buf, "");
    url_entry_t *url_entry;
    while(1)
    {    
        sem_wait(&sem_frontier);
        pthread_mutex_lock(&mutex);
        {
            if(!finished)
            {
                url_entry = STAILQ_FIRST(&url_frontier);
                STAILQ_REMOVE_HEAD(&url_frontier, pointers);
            }
            else
            {
                sem_post(&sem_frontier);
                pthread_mutex_unlock(&mutex);
                break;
            }
            thread_working++;
        }
        pthread_mutex_unlock(&mutex);

        curl_easy_setopt(curl_handle, CURLOPT_URL, url_entry->url);
        free(url_entry);

        curl_easy_perform(curl_handle);

        /* process the download data */
        process_data(curl_handle, &recv_buf);

        pthread_mutex_lock(&mutex);
        {
            thread_working--;
            if((STAILQ_EMPTY(&url_frontier) || image_num == 0) && thread_working == 0)
            {
                finished = 1;
                sem_post(&sem_frontier);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    cleanup(curl_handle, &recv_buf);
    pthread_exit(NULL);
}

void add_url(char *url)
{
    int inserted = 0;
    ENTRY new_url;
    new_url.data = NULL;
    new_url.key = url;
    ENTRY *res;

    url_entry_t *url_entry = (url_entry_t *)malloc(sizeof(url_entry_t));
    
    pthread_mutex_lock(&mutex);
    {
        hsearch_r(new_url, FIND, &res, &visited_urls);
        if(res == NULL)
        {
            strcpy(url_buf[url_buf_tail], url);
            new_url.key = url_buf[url_buf_tail++];
            hsearch_r(new_url, ENTER, &res, &visited_urls);

            strcpy(url_entry->url, url);
            STAILQ_INSERT_TAIL(&url_frontier, url_entry, pointers);
            inserted = 1;
            sem_post(&sem_frontier);
        }
    }
    pthread_mutex_unlock(&mutex);

    if(!inserted)
    {
        free(url_entry);
    }

}

int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    int follow_relative_link = 1;
    char *url = NULL; 

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    find_http(p_recv_buf->buf, p_recv_buf->size, follow_relative_link, url, add_url); 
    return 0;
}

int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    int png = is_png((uint8_t *)p_recv_buf->buf, 8);

    char *url = NULL;          /* effective URL */
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);

    ENTRY new_url;
    new_url.data = NULL;
    new_url.key = url;
    ENTRY *res;
    
    pthread_mutex_lock(&mutex);
    {
        hsearch_r(new_url, FIND, &res, &visited_pngs);
        if(res == NULL)
        {
            strcpy(url_buf[url_buf_tail], url);
            new_url.key = url_buf[url_buf_tail++];
            if(image_num == 0)
            {
                finished = 1;
                sem_post(&sem_frontier);
            }
            else if(png)
            {
                image_num--;
                fprintf(f_result, "%s\n", url);
            }

            hsearch_r(new_url, ENTER, &res, &visited_pngs);
        }
    }
    pthread_mutex_unlock(&mutex);

    return 0;
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

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	;
    } else {
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {
        char *url = NULL;
        curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
        pthread_mutex_lock(&mutex);
        {
            if(f_log != NULL)
            {
                fprintf(f_log, "%s\n", url);
            }  
        }
        pthread_mutex_unlock(&mutex);

        return process_html(curl_handle, p_recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        return process_png(curl_handle, p_recv_buf);
    }

    return 0;
}