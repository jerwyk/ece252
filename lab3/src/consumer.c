#include <semaphore.h>
#include <unistd.h>
#include "processes.h"
#include "zutil.h"
#include "png.h"
#include "queue.h"

#define SEGMENT_SIZE 10000
#define SEGMENT_COUNT 50
#define STRIP_WIDTH 400
#define STRIP_HEIGHT 6
#define STRIP_SIZE STRIP_HEIGHT * (STRIP_WIDTH * 4 + 1)

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#define round_increase(a) a = (a % SERVER_NUM) + 1

/* function declearations */
void consumer(int X, int shmid);

void consumer(int X, int shmid)){
	/* attach shared memory */
	buffer_queue_t* queue = shmat(shmid, NULL, 0);
	void* owo_p = shmat(consumer_shmid, NULL, 0);
	
	int status;
	
	/* reading image segment */
	buffer_item_t* item = malloc(sizeof(buffer_item_t));
	if(is_empty(queue)){
		dequeue(queue, item);
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
	simple_png_p* png_image = init_simple_png();
	status = parse_simple_png(png_image, item->buf);
	if(status != 0){
		printf("Error: parsing png failed\n");
	}
	
	const long BUFFER_SIZE = STRIP_HEIGHT * (STRIP_WIDTH* 4 + 1);
	uint8_t inflated_IDAT_buffer[BUFFER_SIZE];

	status = mem_inf(&inflated_IDAT_buffer, 0, png_image->p_IDAT->p_data, png_image->p_IDAT->length);
	if(status != 0){
		printf("Error: decompressing image failed\n");
	}
	
	/* copying data to shared memory */
	memcpy(owo_p + item->seg_num * STRIP_SIZE, &inflated_IDAT_buffer, BUFFER_SIZE);
	
	/* free memory and detach shared memory */
	free(item);
	/* TODO: item should be only freed if it was malloc'd */
	shmdt(queue);
	return;
}