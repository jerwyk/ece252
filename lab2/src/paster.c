#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>
#include "../../com/png.h"
#include "util.h"

#define ECE252_HEADER "X-Ece252-Fragment: "
#define STRIP_NUM 50
#define BUF_SIZE 1048576  /* 1024*1024 = 1M */
#define BUF_INC  524288   /* 1024*512  = 0.5M */

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define round_increase(a, max) a + 1 > max ? a = 1 : ++a

const char* base_url = "http://ece252-%s.uwaterloo.ca:2520/image?img=%d";

typedef struct recv_buf {
    char *buf;       /* memory to hold a copy of received data */
    size_t size;     /* size of valid data in buf in bytes*/
    size_t max_size; /* max capacity of buf in bytes*/
    int seq;         /* >=0 sequence number extracted from http header */
                     /* <0 indicates an invalid seq number */
} RECV_BUF;

typedef struct thread_arg {
    int server_num;
    int *recv_num;
    char **buf;
    char *url;
} THREAD_ARG;

/* function declearations */
size_t curl_write_data(void *buffer, size_t size, size_t nmemb, void *userp);
size_t curl_header_callback(char *buffer, size_t size, size_t nitems, void *userdata);
int recv_buf_init(RECV_BUF *ptr, size_t max_size);
int recv_buf_cleanup(RECV_BUF *ptr);


size_t curl_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    RECV_BUF* p = (RECV_BUF*) userp;

    if (p->size + realsize + 1 > p->max_size) {/* hope this rarely happens */ 
        /* received data is not 0 terminated, add one byte for terminating 0 */
        size_t new_size = p->max_size + max(BUF_INC, realsize + 1);   
        char *q = realloc(p->buf, new_size);
        if (q == NULL) {
            perror("realloc"); /* out of memory */
            return -1;
        }
        p->buf = q;
        p->max_size = new_size;
    }

    memcpy(p->buf + p->size, buffer, realsize); /*copy data from libcurl*/
    p->size += realsize;
    p->buf[p->size] = 0;

    return realsize;

}

size_t curl_header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    int realsize = size * nitems;
    RECV_BUF *p = userdata;
    
    if (realsize > strlen(ECE252_HEADER) &&
	    strncmp(buffer, ECE252_HEADER, strlen(ECE252_HEADER)) == 0)
    {

        /* extract img sequence number */
	p->seq = atoi(buffer + strlen(ECE252_HEADER));

    }
    return realsize;
}

int recv_buf_init(RECV_BUF *ptr, size_t max_size)
{
    void *p = NULL;
    
    if (ptr == NULL) {
        return 1;
    }

    p = malloc(max_size);
    if (p == NULL) {
	return 2;
    }
    
    ptr->buf = p;
    ptr->size = 0;
    ptr->max_size = max_size;
    ptr->seq = -1;              /* valid seq should be non-negative */
    return 0;
}

int recv_buf_cleanup(RECV_BUF *ptr)
{
    if (ptr == NULL) {
	return 1;
    }
    
    free(ptr->buf);
    ptr->size = 0;
    ptr->max_size = 0;
    return 0;
}

void *curl_api_call(void *ptr)
{
    THREAD_ARG *arg = (THREAD_ARG*) ptr;
    CURL *curl_handle = NULL;
    CURLcode res;

    RECV_BUF recv_buf;
    recv_buf_init(&recv_buf, BUF_SIZE);

    curl_handle = curl_easy_init();
    char url_buf[50];
    sprintf(url_buf, arg->url, arg->server_num);
    if(curl_handle) 
    {
        /* curl settings */
        curl_easy_setopt(curl_handle, CURLOPT_URL, url_buf);
        /* register write call back function to process received data */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data); 
        /* user defined data structure passed to the call back function */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&recv_buf);

        /* register header call back function to process received header data */
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, curl_header_callback); 
        /* user defined data structure passed to the call back function */
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&recv_buf);
        
        /* some servers requires a user-agent field */
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        while(*(arg->recv_num) < STRIP_NUM)
        {
            res = curl_easy_perform(curl_handle);

            if( res != CURLE_OK) 
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            else
            {
                if(arg->buf[recv_buf.seq] == NULL)
                {
                    arg->buf[recv_buf.seq] = malloc(recv_buf.size);
                    memcpy(arg->buf[recv_buf.seq], recv_buf.buf, recv_buf.size);
                    ++(*arg->recv_num);
                } 
                recv_buf_cleanup(&recv_buf);
                recv_buf_init(&recv_buf, BUF_SIZE);
            }         
        }
        recv_buf_cleanup(&recv_buf);
        curl_easy_cleanup(curl_handle);
    }

    return NULL;
}

int main(int argc, char** argv)
{
    /*default params */
    int thread_num = 1;
    int image_num = 1;
    /*read params */
    if(argc > 1)
    {
        for(int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i], "-t") == 0)
            {
                if(i+1 >= argc)
                {
                    printf("Parameter error.\n");
                    return -1;
                }
                thread_num = strtol(argv[++i], NULL, 10);
                if(thread_num < 1)
                {
                    printf("You must have at least 1 thread.\n");
                    return -1;
                }
            }
            else if(strcmp(argv[i], "-n") == 0 && i+1 < argc)
            {
                if(i+1 >= argc)
                {
                    printf("Parameter error.\n");
                    return -1;
                }
                image_num = strtol(argv[++i], NULL, 10);
                if(image_num < 1 || image_num > 3)
                {
                    printf("Image number incorrect.\n");
                    return -2;
                }
            }
        }
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * thread_num);
    char url[50] = {'\0'};
    sprintf(url, base_url, "%d",image_num);

    int server_num = 1;
    char *png_buf[STRIP_NUM] = {NULL};
    int recv_num = 0;

    /* init for http call */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    for(size_t i = 0; i < thread_num; ++i)
    {
        /* create arguments */
        THREAD_ARG arg;
        arg.server_num = server_num;
        round_increase(server_num, 3);
        arg.url = url;
        arg.recv_num = &recv_num;
        arg.buf = png_buf;

        pthread_create(&threads[i], NULL, curl_api_call, (void*) &arg);
    }

    for(size_t i = 0; i < thread_num; ++i)
    {
        pthread_join( threads[i], NULL);
    }

    catpng("all.png", png_buf, STRIP_NUM);

    /* clean up */
    for(int i = 0; i < STRIP_NUM; ++i)
    {
        if(png_buf[i] != NULL)
            free(png_buf[i]);
    }
    curl_global_cleanup();
    return 0;


}
