#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>
#include "png.h"
#include "processes.h"

#define SEM_PROC 1
#define IMAGE_WIDTH 400
#define IMAGE_HEIGHT 300
#define STRIP_NUM 50

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

    int shmid = shmget(IPC_PRIVATE, B * sizeof(buffer_item_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	int consumer_shmid = shmget(IPC_PRIVATE, IMAGE_HEIGHT * (IMAGE_WIDTH * 4 + 1), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	/* TODO: find out what size lmao */
    pthread_mutex_t *mutex;
    sem_t *items, *spaces;
    pthread_mutex_init(mutex, NULL);
    sem_init(items, SEM_PROC, B);
    sem_init(spaces, SEM_PROC, B);

    int prod_start = 0;
    int prod_num = (STRIP_NUM / P) + 1;

	for(int i = 1; i <= (P + C); ++i){
		if(fork() == 0){
			break;
		}
		if(i <= P){
			p_producer(N, shmid, prod_start, prod_start + prod_num, mutex, items, spaces);
            prod_start = prod_start + prod_num > STRIP_NUM ? STRIP_NUM : prod_start + prod_num;
		}else if(i <= P + C){
			consumer(X, shmid, consumer_shmid);
		}
    }

	shmctl(consumer_shmid);
	shmctl(shmid);
    curl_global_cleanup();
    return 0;
}
