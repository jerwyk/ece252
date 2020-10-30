#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <semaphore.h>
#include <pthread.h>
#include "queue.h"

void p_producer(int num, int shmid, pthread_mutex_t *mutex, sem_t *items, sem_t *spaces);
void p_consumer(int X, int shmid, int consumer_shmid, pthread_mutex_t *mutex, sem_t *items, sem_t *spaces);