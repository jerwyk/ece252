#include "util.h"
#define IMAGE_WIDTH 400
#define IMAGE_HEIGHT 300

int squeeze(const char* dest, uint8_t* buffer){ /* buffer is the uncompressed data */
	uint64_t compressed_length;	
	const long BUFFER_LENGTH = IMAGE_HEIGHT * (IMAGE_WIDTH * 4 + 1);
	uint8_t dest_buffer[BUFFER_LENGTH]; /* buffer for compressed data */

	/* data compression */
    status = mem_def(dest_buffer, &compressed_length, *buffer, BUFFER_LENGTH, Z_DEFAULT_COMPRESSION);
	if(status != 0){
			printf("Compression error %d\n", status);
			return status;
	}

	/* static data */
    uint8_t PNG_signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	uint8_t IHDR_length[4] = {0x00, 0x00, 0x00, 0x0d};
	uint8_t IHDR_chunk_type[4] = {0x49, 0x48, 0x44, 0x52};
	uint8_t IHDR_data_no_dims[5] = {0x08, 0x06, 0x00, 0x00, 0x00};
	uint8_t IDAT_chunk_type[4] = {0x49, 0x44, 0x41, 0x54};
	uint8_t IEND[12] = {0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
	
	/* prepping data for crc */
	uint32_t width_network = htonl(IMAGE_WIDTH);
	uint32_t height_network = htonl(IMAGE_HEIGHT);
	uint32_t length_network = htonl(compressed_length);
	
	/* writing to IHDR buffer */
	uint8_t IHDR_buffer[17];
	memcpy(IHDR_buffer, IHDR_chunk_type, 4);
	memcpy(IHDR_buffer + 4, &width_network, 4);
	memcpy(IHDR_buffer + 8, &height_network, 4);
	memcpy(IHDR_buffer + 12, IHDR_data_no_dims, 5);
	uint32_t IHDR_crc = crc(IHDR_buffer, 17);
	
	/* writing to IDAT buffer */
	uint8_t IDAT_buffer[4 + compressed_length];
	memcpy(IDAT_buffer, IDAT_chunk_type, 4);
	memcpy(IDAT_buffer + 4, dest_buffer, compressed_length);
	uint32_t IDAT_crc = crc(IDAT_buffer, 4 + compressed_length);
	
	/* crc */
	uint32_t IHDR_crc_network = htonl(IHDR_crc);
	uint32_t IDAT_crc_network = htonl(IDAT_crc);

	/* writing to file */
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
