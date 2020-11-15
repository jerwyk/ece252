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
extern FILE *f_log;
extern FILE *f_result;

extern char url_buf[2048][256];
extern int url_buf_tail;
extern struct url_queue_t url_frontier;

extern pthread_mutex_t mutex;
extern pthread_mutex_t url_mutex;
extern sem_t sem_frontier;
extern pthread_cond_t cond_frontier;
extern int num_thread_wait;

extern int finished;

int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf);

void* t_crawler(void* param)
{
    url_entry_t *url_entry;
    while(1)
    {
        //printf("mutex: %lu\n", pthread_self());
        sem_wait(&sem_frontier);
        pthread_mutex_lock(&mutex);
        {
            if(!finished)
            {
                url_entry = STAILQ_FIRST(&url_frontier);
                //printf("remove: %lu\n", pthread_self());
                STAILQ_REMOVE_HEAD(&url_frontier, pointers);
            }
            
        }
        pthread_mutex_unlock(&mutex);

        if(finished == 1)
        {
            pthread_exit(NULL);
        }

        CURL *curl_handle;
        RECV_BUF recv_buf;

        curl_handle = easy_handle_init(&recv_buf, url_entry->url);
        curl_easy_perform(curl_handle);

        /* process the download data */
        process_data(curl_handle, &recv_buf);

        pthread_mutex_lock(&mutex);
        {
            num_thread_wait++;
            if(STAILQ_EMPTY(&url_frontier) || image_num == 0)
            {
                if(num_thread_wait < thread_num && image_num != 0)
                {
                    pthread_cond_wait(&cond_frontier, &mutex);
                }
                else
                {
                    finished = 1;
                    pthread_cond_broadcast(&cond_frontier);
                    sem_post(&sem_frontier);
                } 
            }
            num_thread_wait--;
        }
        pthread_mutex_unlock(&mutex);
    }
}

void add_url(char *url)
{
    ENTRY new_url;
    new_url.data = NULL;
    new_url.key = url;
    ENTRY *res;
    
    pthread_mutex_lock(&mutex);
    {
        res = hsearch(new_url, FIND);
        if(res == NULL)
        {
            url_entry_t *url_entry = (url_entry_t *)malloc(sizeof(url_entry_t));
            strcpy(url_entry->url, url);
            STAILQ_INSERT_TAIL(&url_frontier, url_entry, pointers);
            //printf("signal: %lu\n", pthread_self());
            pthread_cond_signal(&cond_frontier);
            sem_post(&sem_frontier);
        }
    }
    pthread_mutex_unlock(&mutex);

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
        res = hsearch(new_url, FIND);
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

            hsearch(new_url, ENTER);
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
        //fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {
        char *url = NULL;
        curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
        ENTRY new_url;
        new_url.data = NULL;
        new_url.key = url;
        ENTRY *hres;  
        pthread_mutex_lock(&url_mutex);
        {
            hres = hsearch(new_url, FIND);
            if(hres == NULL)
            {
                strcpy(url_buf[url_buf_tail], url);
                new_url.key = url_buf[url_buf_tail++]; 
                if(f_log != NULL)
                {
                    fprintf(f_log, "%s\n", url);
                    printf("%s\n", url);
                }  
                hsearch(new_url, ENTER);
            }
        }
        pthread_mutex_unlock(&url_mutex);

        return process_html(curl_handle, p_recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        return process_png(curl_handle, p_recv_buf);
    }

    return 0;
}