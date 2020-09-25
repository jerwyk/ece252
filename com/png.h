#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

/******************************************************************************
 * DEFINED MACROS 
 *****************************************************************************/

#define PNG_SIG_SIZE    8 /* number of bytes of png image signature data */
#define CHUNK_LEN_SIZE  4 /* chunk length field size in bytes */          
#define CHUNK_TYPE_SIZE 4 /* chunk type field size in bytes */
#define CHUNK_CRC_SIZE  4 /* chunk CRC field size in bytes */
#define DATA_IHDR_SIZE 13 /* IHDR chunk data field size */

#define IHDR_CHUNK_SIZE CHUNK_LEN_SIZE+CHUNK_TYPE_SIZE+CHUNK_CRC_SIZE+DATA_IHDR_SIZE

typedef unsigned char uint8_t;
typedef unsigned int  uint32_t;

typedef struct chunk {
    uint32_t length;  /* length of data in the chunk, host byte order */
    uint8_t  type[4]; /* chunk type */
    uint8_t  *p_data; /* pointer to location where the actual data are */
    uint32_t crc;     /* CRC field  */
} *chunk_p;

/* note that there are 13 Bytes valid data, compiler will padd 3 bytes to make
   the structure 16 Bytes due to alignment. So do not use the size of this
   structure as the actual data size, use 13 Bytes (i.e DATA_IHDR_SIZE macro).
 */
typedef struct data_IHDR {// IHDR chunk data 
    uint32_t width;        /* width in pixels, big endian   */
    uint32_t height;       /* height in pixels, big endian  */
    uint8_t  bit_depth;    /* num of bits per sample or per palette index.
                         valid values are: 1, 2, 4, 8, 16 */
    uint8_t  color_type;   /* =0: Grayscale; =2: Truecolor; =3 Indexed-color
                         =4: Greyscale with alpha; =6: Truecolor with alpha */
    uint8_t  compression;  /* only method 0 is defined for now */
    uint8_t  filter;       /* only method 0 is defined for now */
    uint8_t  interlace;    /* =0: no interlace; =1: Adam7 interlace */
} *data_IHDR_p;

typedef struct simple_PNG {
    struct chunk *p_IHDR;
    struct chunk *p_IDAT;  /* only handles one IDAT chunk */  
    struct chunk *p_IEND;
} *simple_PNG_p;

//returns 1 if it is an png
int is_png(uint8_t *buf, size_t n);

int get_png_height(struct data_IHDR *buf);
int get_png_width(struct data_IHDR *buf);

//reads data in a png file. Returns -1 if the file is not a png
//it does not check for corrupted files
int read_simple_png(simple_PNG_p png, FILE* fptr);
