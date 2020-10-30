#include <semaphore.h>
#include <unistd.h>
#include "processes.h"
#include "zutil.h"
#include "png.h"
#include "queue.h"

#define STRIP_COUNT 50
#define STRIP_WIDTH 400
#define STRIP_HEIGHT 6
#define BUFFER_SIZE STRIP_HEIGHT * (STRIP_WIDTH * 4 + 1)

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#define round_increase(a) a = (a % SERVER_NUM) + 1

void p_consumer(int X, int shmid, int consumer_shmid, pthread_mutex_t *mutex,pthread_mutex_t *con_mutex, sem_t *items, sem_t *spaces){
	/* attach shared memory */
	buffer_queue_t* queue = shmat(shmid, NULL, 0);
	void* png_buffer = shmat(consumer_shmid, NULL, 0);
	
	int status;
	int exit = 0;
	buffer_item_t* item = malloc(sizeof(buffer_item_t));
	
	while(!exit){
		/* reading image segment */
		
		/* critical section */
		pthread_mutex_lock(con_mutex);
		if(queue->counter > 0){
			sem_wait(items);
			pthread_mutex_lock(mutex);
			dequeue(queue, item);
			--(queue->counter);
			pthread_mutex_unlock(mutex);
			sem_post(spaces);
		}else{
			exit = 1;
		}
		pthread_mutex_unlock(con_mutex);

		if(exit){
			break;
		}

		/* sleep for X milliseconds */
		usleep(X * 1000);
		/* processing received data */	
		/* validating png */
		if(!is_png((uint8_t*)(item->buf), PNG_SIG_SIZE)){
			printf("Error: not a valid image\n");
			return;
		}

		/* inflating IDAT data */
		simple_PNG_p png_image = init_simple_png();
		status = parse_simple_png(png_image, item->buf);
		if(status != 0){
			printf("Error: parsing png failed\n");
			return;
		}
		
		/* copying data to shared memory */
		/* not a critical section becasue it always writes to a different area of memory */
		status = mem_inf(png_buffer + item->seg_num * BUFFER_SIZE, NULL, png_image->p_IDAT->p_data, png_image->p_IDAT->length);
		if(status != 0){
			printf("Error: decompressing image failed\n");
			return;
		}

		free_simple_png(png_image);

	}
	
	/* freeing dynamically allocated memory and detaching from shared memory */
	free(item);
	shmdt(png_buffer);
	shmdt(queue);
	return;
}