#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>
#include "png.h"
//#include "util.h"

#define ECE252_HEADER "X-Ece252-Fragment: "
#define SERVER_NUM 3
#define STRIP_NUM 50
#define BUF_SIZE 1048576  /* 1024*1024 = 1M */
#define BUF_INC  524288   /* 1024*512  = 0.5M */

typedef struct thread_arg {
    int server_num;
    int *recv_num;
    char **buf;
    char *url;
} THREAD_ARG;


int main(int argc, char** argv)
{
    /* init for http call */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    /*read params and error checking*/
    if(argc != 6)
    {
        printf("Invalid arguments");
        return -1;
    }

    /* get parameters */
    int B, P, C, X, N;
    B = strtol(argv[1], NULL, 10);
    P = strtol(argv[2], NULL, 10);
    C = strtol(argv[3], NULL, 10);
    X = strtol(argv[4], NULL, 10);
    N = strtol(argv[5], NULL, 10);

    

    curl_global_cleanup();
    return 0;
}
