#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../com/png.h"
#include "./starter/png_util/zutil.h"
#include "./starter/png_util/crc.h"

#define BUF_LEN (2560*64)
#define BUF_LEN_DEF (256*64)

int write_chunk(chunk_p c_p, FILE *fptr);

int main(int argc, char *argv[]) 
{
    if(argc == 1)
    {
        printf("Concatenate PNG images vertically to create a new PNG named \"all.png\"");
        exit(0);
    }

    FILE *fptr;
    fptr = fopen(argv[1], "r");
    simple_PNG_p all_png = NULL;
    read_simple_png(&all_png, fptr);
    free(all_png->p_IDAT->p_data);
    all_png->p_IDAT->p_data = NULL;

    uint8_t all_inf_data[BUF_LEN*(argc - 1)];
    uint64_t all_inf_length = 0;
    uint32_t all_height = 0;
    simple_PNG_p png_p = NULL;

    for (int i = 1; i < argc; i++)
    {      
        fptr = fopen(argv[i], "r");
        if(read_simple_png(&png_p, fptr) < 0)
        {
            fprintf(stderr, "Invalid PNG\n");
            exit(1);
        }
        uint64_t inf_len;
        mem_inf(all_inf_data + all_inf_length, &inf_len, png_p->p_IDAT->p_data, png_p->p_IDAT->length);
        all_inf_length += inf_len;

        all_height += ((data_IHDR_p)png_p->p_IHDR->p_data)->height;

        fclose(fptr);
    }
    free_simple_png(png_p);
    png_p = NULL;

    uint64_t def_len;
    uint8_t all_def_data[BUF_LEN_DEF*(argc - 1)];
    memcpy(all_def_data, all_png->p_IDAT->type, CHUNK_TYPE_SIZE);
    mem_def(all_def_data + CHUNK_TYPE_SIZE, &def_len, all_inf_data, all_inf_length, Z_DEFAULT_COMPRESSION);

    all_png->p_IDAT->crc = crc(all_def_data, def_len + CHUNK_TYPE_SIZE);
    all_png->p_IDAT->length = (uint32_t)def_len;
    all_png->p_IDAT->p_data = all_def_data + CHUNK_TYPE_SIZE;

    ((data_IHDR_p)all_png->p_IHDR->p_data)->height = all_height;
    uint8_t *IHDR_buf = malloc(CHUNK_TYPE_SIZE + DATA_IHDR_SIZE);
    memcpy(IHDR_buf, all_png->p_IHDR->type, CHUNK_TYPE_SIZE);
    memcpy(IHDR_buf + CHUNK_TYPE_SIZE, all_png->p_IHDR->p_data, DATA_IHDR_SIZE);
    all_png->p_IHDR->crc = crc(IHDR_buf, CHUNK_TYPE_SIZE + DATA_IHDR_SIZE);

    /*write to file*/
    FILE *all_fptr = fopen("all.png", "wb");
    /*write signature*/
    uint8_t signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    fwrite(signature, sizeof(uint8_t), sizeof signature, all_fptr);

    write_chunk(all_png->p_IHDR, all_fptr);
    write_chunk(all_png->p_IDAT, all_fptr);
    write_chunk(all_png->p_IEND, all_fptr);

    fclose(all_fptr);
    all_png->p_IDAT->p_data = NULL;
    free_simple_png(all_png);
    free(IHDR_buf);

    return 0;
}

int write_chunk(chunk_p c_p, FILE *fptr)
{
    /*write length*/
    uint32_t n_len = htonl(c_p->length);
    fwrite(&n_len, CHUNK_LEN_SIZE, 1, fptr);

    fwrite(c_p->type, 1, CHUNK_TYPE_SIZE, fptr);

    fwrite(c_p->p_data, c_p->length, 1, fptr);

    uint32_t n_crc = htonl(c_p->crc);
    fwrite(&n_crc, CHUNK_CRC_SIZE, 1, fptr);

    return 0;
}