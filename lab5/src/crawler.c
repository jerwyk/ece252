#define _GNU_SOURCE

#include "crawler.h"
#include "util.h"
#include "png.h"
#include <search.h>

#define CT_PNG  "image/png"
#define CT_HTML "text/html"
#define BUF_SIZE 1048576  /* 1024*1024 = 1M */

typedef struct hsearch_data hashmap_t;

extern int image_num;
extern FILE *f_log;
extern FILE *f_result;

extern char url_buf[2048][256];
extern int url_buf_tail;
extern struct url_queue_t url_frontier;
extern struct hsearch_data visited_urls;
extern struct hsearch_data visited_pngs;

extern int finished;

int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf);
int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf);

size_t crawler_cb(char *p_recv, size_t size, size_t nmemb, void *p_userdata)
{
    RECV_BUF p;
    CURL *h = (CURL *)p_userdata;
    p.size = size * nmemb;
    p.buf = p_recv;
    process_data(h, &p);

    return p.size;
}

void add_url(char *url)
{
    int inserted = 0;
    ENTRY new_url;
    new_url.data = NULL;
    new_url.key = url;
    ENTRY *res;

    url_entry_t *url_entry = (url_entry_t *)malloc(sizeof(url_entry_t));
    
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
        }
    }

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
    
    hsearch_r(new_url, FIND, &res, &visited_pngs);
    if(res == NULL)
    {
        strcpy(url_buf[url_buf_tail], url);
        new_url.key = url_buf[url_buf_tail++];
        if(image_num == 0)
        {
            strcpy(url_buf[url_buf_tail], url);
            new_url.key = url_buf[url_buf_tail++];
            if(image_num == 0)
            {
                finished = 1;
            }
            else if(png)
            {
                image_num--;
                fprintf(f_result, "%s\n", url);
            }

            hsearch_r(new_url, ENTER, &res, &visited_pngs);
        }

        hsearch_r(new_url, ENTER, &res, &visited_pngs);
    }

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
    long response_code;

    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

    char *url = NULL;
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    {
        if(f_log != NULL)
        {
            fprintf(f_log, "%s\n", url);
        }  
    }

    if ( response_code >= 400 ) 
    { 
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	;
    } else {
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {
        return process_html(curl_handle, p_recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        return process_png(curl_handle, p_recv_buf);
    }

    return 0;
}