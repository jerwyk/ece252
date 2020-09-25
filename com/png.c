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
    return __bswap_32(buf->height);
}

int get_png_width(struct data_IHDR *buf)
{
    return __bswap_32(buf->width);
}