#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../com/png.h"
#include "./starter/png_util/crc.h"

int main(int argc, char *argv[]) 
{
    if(argc == 2) 
    {
        char* path = argv[1];
        FILE *fptr;

        if ((fptr = fopen(path,"r")) != NULL)
        {
            simple_PNG_p png_p;
            if(read_simple_png(&png_p, fptr) == 0)
            {
                printf("%s: %d x %d\n", path, get_png_width((data_IHDR_p)png_p->p_IHDR->p_data), get_png_height((data_IHDR_p)png_p->p_IHDR->p_data));

                uint8_t *IHDR_buf = malloc(CHUNK_TYPE_SIZE + DATA_IHDR_SIZE);
                memcpy(IHDR_buf, png_p->p_IHDR->type, CHUNK_TYPE_SIZE);
                memcpy(IHDR_buf + CHUNK_TYPE_SIZE, png_p->p_IHDR->p_data, DATA_IHDR_SIZE);

                uint8_t *IDAT_buf = malloc(CHUNK_TYPE_SIZE + png_p->p_IDAT->length);
                memcpy(IDAT_buf, png_p->p_IDAT->type, CHUNK_TYPE_SIZE);
                memcpy(IDAT_buf + CHUNK_TYPE_SIZE, png_p->p_IDAT->p_data, png_p->p_IDAT->length);
                
                unsigned long ihdr_calc_crc = crc(IHDR_buf, CHUNK_TYPE_SIZE + DATA_IHDR_SIZE);
                if(ihdr_calc_crc != png_p->p_IHDR->crc)
                {
                    printf("IHDR chunk CRC error: computed %lx, expected %x\n", ihdr_calc_crc, png_p->p_IHDR->crc);
                }
                unsigned long idat_calc_crc = crc(IDAT_buf, CHUNK_TYPE_SIZE + png_p->p_IDAT->length);
                if(idat_calc_crc != png_p->p_IDAT->crc)
                {
                    printf("IDAT chunk CRC error: computed %lx, expected %x\n", idat_calc_crc, png_p->p_IDAT->crc);
                }

                unsigned long iend_calc_crc = crc(png_p->p_IEND->type, CHUNK_TYPE_SIZE);
                if(iend_calc_crc != png_p->p_IEND->crc)
                {
                    printf("IEND chunk CRC error: computed %lx, expected %x\n", iend_calc_crc, png_p->p_IEND->crc);
                }

                free_simple_png(png_p);
                free(IHDR_buf);
                free(IDAT_buf);
                

            }
            else
            {
                printf("%s: Not a PNG file", path);
            }

            fclose(fptr);
            
        }
        else
        {
            printf("Error when opening file. Please check the path\n");
        }
    }
}   