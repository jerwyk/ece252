#include "util.h"

int catpng(const char* dest, char **pngs, size_t num)
{
    int all_height = 0;
	int all_width, status;
	uint64_t compressed_length;
    simple_PNG_p png_image = init_simple_png();

    for(int i = 0; i < num; ++i)
    {		
		parse_simple_png(png_image, pngs[i]);
		all_height += get_png_height((data_IHDR_p)png_image->p_IHDR->p_data); /* calculates total height of all.png */
		if(i != 0)
        { /* error check if all the files are the same width */
			if(all_width != get_png_width((data_IHDR_p)png_image->p_IHDR->p_data))
            {
				printf("Images are not the same width!\n");
				return -1;
			}
		}
		all_width = get_png_width((data_IHDR_p)png_image->p_IHDR->p_data);
	}
	
	const long BUFFER_LENGTH = all_height * (all_width * 4 + 1);
	uint8_t buffer[BUFFER_LENGTH]; /* buffer for decompressed data */
	uint8_t dest_buffer[BUFFER_LENGTH]; /* buffer for compressed data */
	uint8_t* buffer_p = buffer;
		
	for(int i = 0; i < num; ++i)
    {
		uint64_t buffer_offset = 0;
        parse_simple_png(png_image, pngs[i]);
		status = mem_inf(buffer_p, &buffer_offset, png_image->p_IDAT->p_data, png_image->p_IDAT->length);
		if(status != 0){
			printf("Decompression error %d\n", status);
			return status;
		}
		buffer_p += buffer_offset;		
	}
	
    status = mem_def(dest_buffer, &compressed_length, buffer, BUFFER_LENGTH, Z_DEFAULT_COMPRESSION);
	if(status != 0){
			printf("Compression error %d\n", status);
			return status;
	}

    uint8_t PNG_signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	uint8_t IHDR_length[4] = {0x00, 0x00, 0x00, 0x0d};
	uint8_t IHDR_chunk_type[4] = {0x49, 0x48, 0x44, 0x52};
	uint8_t IHDR_data_no_dims[5] = {0x08, 0x06, 0x00, 0x00, 0x00};
	uint8_t IDAT_chunk_type[4] = {0x49, 0x44, 0x41, 0x54};
	uint8_t IEND[12] = {0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
	
	uint32_t width_network = htonl(all_width);
	uint32_t height_network = htonl(all_height);
	uint32_t length_network = htonl(compressed_length);
	
	uint8_t IHDR_buffer[17];
	memcpy(IHDR_buffer, IHDR_chunk_type, 4);
	memcpy(IHDR_buffer + 4, &width_network, 4);
	memcpy(IHDR_buffer + 8, &height_network, 4);
	memcpy(IHDR_buffer + 12, IHDR_data_no_dims, 5);
	uint32_t IHDR_crc = crc(IHDR_buffer, 17);

	
	uint8_t IDAT_buffer[4 + compressed_length];
	memcpy(IDAT_buffer, IDAT_chunk_type, 4);
	memcpy(IDAT_buffer + 4, dest_buffer, compressed_length);
	uint32_t IDAT_crc = crc(IDAT_buffer, 4 + compressed_length);
	
	uint32_t IHDR_crc_network = htonl(IHDR_crc);
	uint32_t IDAT_crc_network = htonl(IDAT_crc);

    FILE* dest_image = fopen(dest, "wb+");
	
	fwrite(&PNG_signature, 1, 8, dest_image);
	fwrite(&IHDR_length, 1, 4, dest_image);
	fwrite(&IHDR_chunk_type, 1, 4, dest_image);
	fwrite(&width_network, 4, 1, dest_image);
	fwrite(&height_network, 4, 1, dest_image);
	fwrite(&IHDR_data_no_dims, 1, 5, dest_image);
	fwrite(&IHDR_crc_network, 4, 1, dest_image);
	fwrite(&length_network, 4, 1, dest_image);
	fwrite(&IDAT_chunk_type, 1, 4, dest_image);
	fwrite(&dest_buffer, 1, compressed_length, dest_image);
	fwrite(&IDAT_crc_network, 4, 1, dest_image);
	fwrite(&IEND, 1, 12, dest_image);
	
	fclose(dest_image);
	
	return 0;

}