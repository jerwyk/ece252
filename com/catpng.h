#include "crc.h"      /* for crc()                   */
#include "zutil.h"    /* for mem_def() and mem_inf() */
#include "lab_png.h"  /* simple PNG data structures  */

#define BUF_LEN  (256*16)
#define BUF_LEN2 (256*32)

#define PNG_SIGNATURE	89504e470d0a1a0a
#define IHDR_LENGTH 			0000000d
#define IHDR_CHUNK_TYPE 		49484452
#define IHDR_CHUNK_DATA		  0806000000
#define IDAT_CHUNK_TYPE			49444154
#define IEND 	0000000049454e44ae426082

typedef unsigned int crc;

data_IHDR_p* get_IHDR_data(char* directory);
uint32_t get_IDAT_length(char* directory);
uint8_t* get_IDAT_data(char* directory);
int cat_raw_data(int image_count, char* dirs[]);
int generate_output(int width, int height);