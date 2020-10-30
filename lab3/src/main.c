#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <pthread.h>
#include <sys/time.h>
#include "png.h"
#include "processes.h"
#include "util.h"

#define SEM_PROC 1
#define IMAGE_WIDTH 400
#define IMAGE_HEIGHT 300
#define STRIP_COUNT 50

typedef struct concurrency {
    pthread_mutex_t mutex;
    pthread_mutex_t con_mutex;
    sem_t items;
    sem_t spaces;
}concurrency_t;


int main(int argc, char** argv)
{
    /* used for program timing */
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

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

    int shmid = shmget(IPC_PRIVATE, sizeof_shm_queue(B), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	int consumer_shmid = shmget(IPC_PRIVATE, IMAGE_HEIGHT * (IMAGE_WIDTH * 4 + 1), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int shmid_sems = shmget(IPC_PRIVATE, sizeof(concurrency_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	
	buffer_queue_t* queue_init = shmat(shmid, NULL, 0);
    concurrency_t *sems = shmat(shmid_sems, NULL, 0);
	init_shm_queue(queue_init, B);

    pthread_mutexattr_t attrmutex;
    /* Initialise attribute to mutex. */
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sems->mutex, &attrmutex);
    pthread_mutex_init(&sems->con_mutex, &attrmutex);


    if ( sem_init(&sems->items, SEM_PROC, 0) != 0 ) {
        perror("sem_init(items)");
        abort();
    }
    if ( sem_init(&sems->spaces, SEM_PROC, B) != 0 ) {
        perror("sem_init(spaces)");
        abort();
    }

	for(int i = 1; i <= (P + C); ++i){
		if(fork() == 0){
            if(i <= P){
                p_producer(N, shmid, &sems->mutex, &sems->items, &sems->spaces);
                exit(0);
            }else if(i <= (P + C)){
                p_consumer(X, shmid, consumer_shmid, &sems->mutex, &sems->con_mutex, &sems->items, &sems->spaces);
                exit(0);
            }
        }
    }

    /* wait for all child to exit */
    pid_t wpid;
    while ((wpid = wait(NULL)) > 0);
    uint8_t *all_data = (uint8_t *)shmat(consumer_shmid, NULL, 0);
    squeeze("all.png", all_data);

    //destroy_queue(queue_init);
	shmctl(consumer_shmid, IPC_RMID, NULL);
	shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmid_sems, IPC_RMID, NULL);
    pthread_mutex_destroy(&sems->mutex);
    pthread_mutex_destroy(&sems->con_mutex);
    sem_destroy(&sems->items);
    sem_destroy(&sems->spaces);
    curl_global_cleanup();

    gettimeofday(&t2, NULL);
    double t1_sec, t2_sec;
    t1_sec = t1.tv_sec + t1.tv_usec / 1000000.0;
    t2_sec = t2.tv_sec + t2.tv_usec / 1000000.0;

    printf("paster2 execution time: %f seconds\n", t2_sec - t1_sec);
    return 0;
}
