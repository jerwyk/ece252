#include <stdio.h>
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
            simple_PNG_p png_p = malloc(sizeof(struct simple_PNG));
            if(read_simple_png(png_p, fptr) == 0)
            {
                printf("%s: %d x %d\n", path, get_png_width(png_p->p_IHDR->p_data), get_png_height(png_p->p_IHDR->p_data));
                uint8_t 
            }
            else
            {
                printf("%s: Not a PNG file", path);
            }
            
        }
        else
        {
            printf("Error when opening file. Please check the path\n");
        }
    }
}   