#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include "../com/png.h"

int main(int argc, char *argv[]) 
{
    if(argc == 2) 
    {
        char* path = argv[1];
        FILE *fptr;

        if ((fptr = fopen(path,"r")) != NULL)
        {
            uint8_t sig[PNG_SIG_SIZE];
            fread(&sig, sizeof(sig), 1, fptr);
            if(is_png(sig, PNG_SIG_SIZE))
            {
                chunk_p ihdr_chunk_p = malloc(sizeof(struct chunk));
                fread(ihdr_chunk_p, sizeof(struct chunk), 1, fptr);
                data_IHDR_p ihdr_p = ihdr_chunk_p->p_data;
                int height = get_png_height(ihdr_p);
                int width = get_png_width(ihdr_p);
                printf("%s: %d x %d\n", path, width, height);
            }
            else
            {
                printf("This file is not a png\n");
            }
            
        }
        else
        {
            printf("Error when opening file. Please check the path\n");
        }
    }
}   