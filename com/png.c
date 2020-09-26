#include "png.h"

int is_png(uint8_t *buf, size_t n)
{
    uint8_t signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    for(int i = 0; i < n; ++i)
    {
        if(buf[i] != signature[i])
        {
            return 0;
        }
    } 

    return 1;
}

int get_png_height(struct data_IHDR *buf)
{
    return ntohl(buf->height);
}

int get_png_width(struct data_IHDR *buf)
{
    return ntohl(buf->width);
}

int read_simple_png(simple_PNG_p png, FILE* fptr)
{
    png = malloc(sizeof(struct simple_PNG));
    /*signature of file*/
    uint8_t sig[PNG_SIG_SIZE];
    fread(&sig, sizeof(sig), 1, fptr);
    if(is_png(sig, PNG_SIG_SIZE))
    {   
        /*IHDR chunk*/
        /*alloc space for ihdr*/
        png->p_IHDR = malloc(sizeof(struct chunk));
        png->p_IHDR->p_data = malloc(sizeof(uint8_t));
        /*read length of chunk*/
        uint32_t len_buf = 0;
        fread(&len_buf, CHUNK_LEN_SIZE, 1, fptr);
        png->p_IHDR->length = ntohl(len_buf);
        /*read type of chunk*/
        fread(&(png->p_IHDR->type), CHUNK_TYPE_SIZE, 1, fptr);
        /*read IHDR data*/
        uint8_t *ihdr_p = malloc(DATA_IHDR_SIZE);
        fread(ihdr_p, DATA_IHDR_SIZE, 1, fptr);
        png->p_IHDR->p_data = ihdr_p;
        /*read crc value*/
        fread(&(png->p_IHDR->crc), CHUNK_CRC_SIZE, 1, fptr);
        png->p_IHDR->crc = ntohl(png->p_IHDR->crc);

        /*IDAT chunk*/
        /*alloc space for idat*/
        png->p_IDAT = malloc(sizeof(struct chunk));;
        png->p_IDAT->p_data = malloc(sizeof(uint8_t));
        /*read length of chunk*/
        fread(&len_buf, CHUNK_LEN_SIZE, 1, fptr);
        png->p_IDAT->length = ntohl(len_buf);
        /*read type of chunk*/
        fread(&(png->p_IDAT->type), CHUNK_TYPE_SIZE, 1, fptr);
        /*read data*/
        uint8_t *idat_data_p = malloc(png->p_IDAT->length);
        fread(idat_data_p, png->p_IDAT->length, 1, fptr);
        png->p_IDAT->p_data = idat_data_p;
        /*read crc value*/
        fread(&(png->p_IDAT->crc), CHUNK_CRC_SIZE, 1, fptr);
        png->p_IDAT->crc = ntohl(png->p_IDAT->crc);

        /*IEND chunk*/
        /*alloc space for iend*/
        png->p_IEND = malloc(sizeof(struct chunk));
        /*no data for iend*/
        png->p_IEND->p_data = NULL;
        /*read length of chunk*/
        fread(&len_buf, CHUNK_LEN_SIZE, 1, fptr);
        png->p_IEND->length = ntohl(len_buf);
        /*read type of chunk*/
        fread(&(png->p_IEND->type), CHUNK_TYPE_SIZE, 1, fptr);
        /*no data for iend*/
        /*read crc*/
        fread(&(png->p_IEND->crc), CHUNK_CRC_SIZE, 1, fptr);
        png->p_IEND->crc = ntohl(png->p_IEND->crc);

        return 0;
    }
    else
    {
        return -1;
    }
        
}

int free_simple_png(simple_PNG_p png)
{
    free(png->p_IHDR->p_data);
    free(png->p_IHDR);

    free(png->p_IDAT->p_data);
    free(png->p_IDAT);

    free(png->p_IEND);

    free(png);

    return 0;
}