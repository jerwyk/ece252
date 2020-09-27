#include "../com/png.h"
#include "./starter/png_util/crc.h"
#include "./starter/png_util/zutil.h"

#define BUF_LEN  (256*16)
#define BUF_LEN2 (256*32)

#define PNG_SIGNATURE   "89504e470d0a1a0a"
#define IHDR_LENGTH             "0000000d"
#define IHDR_CHUNK_TYPE 		"49484452"
#define IHDR_CHUNK_DATA       "0806000000"
#define IDAT_CHUNK_TYPE         "49444154"
#define IEND    "0000000049454e44ae426082"

data_IHDR_p get_IHDR_data(char* directory);
uint32_t get_IDAT_length(char* directory);
uint8_t* get_IDAT_data(char* directory);
U64 cat_raw_data(int image_count, char* dirs[]);
int generate_output(int width, int height, int IDAT_length);

data_IHDR_p get_IHDR_data(char* directory){
	data_IHDR_p result;
	simple_PNG_p png_image;
	
	FILE* png_file = fopen(directory, "rb");
	read_simple_png(png_image, png_file);
	result = png_image->p_IHDR->p_data;
	fclose(png_file);
	return result;
}

uint32_t get_IDAT_length(char* directory){
	uint32_t result;
	simple_PNG_p png_image;
	
	FILE* png_file = fopen(directory, "rb");
	read_simple_png(png_image, png_file);
	result = png_image->p_IDAT->length;
	fclose(png_file);
	return result;
}

uint8_t* get_IDAT_data(char* directory){
	uint8_t* result;
	simple_PNG_p png_image;
	FILE* png_file = fopen(directory, "rb");
	
	read_simple_png(png_image, png_file);
	result = png_image->p_IDAT->p_data;
	fclose(png_file);
	return result;
}

U64 cat_raw_data(int image_count, char* dirs[]){
	FILE* temp = fopen("temp", "wb+");
	U64 file_size = 0;
	U64 total_file_size = 0;
	for(int i = 1; i <= image_count; ++i){
		uint8_t* buffer = malloc(sizeof(uint8_t));
		buffer = get_IDAT_data(dirs[i]);
		mem_inf(temp, &file_size, buffer, get_IDAT_length(dirs[i]));
		free(buffer);
		total_file_size += file_size;
	}
	FILE* output = fopen("output", "wb+");
	void* buffer = malloc(total_file_size);
	fread(buffer, 1, total_file_size, temp);
	mem_def(output, file_size, buffer, total_file_size, Z_DEFAULT_COMPRESSION);
	fclose(temp);
	fclose(output);
	free(buffer);
	return file_size;
}

void fprintf_checksum(int offset, int length, FILE* filename){
	void* buffer = malloc(length);
	fseek(filename, offset, SEEK_SET);
	fread(buffer, 1, length, filename);
	fprintf(filename, "%08x", crc(buffer, length));
	free(buffer);
}

int generate_output(int width, int height, int IDAT_length){
	FILE* result = fopen("all.png", "wb+");
	FILE* IDAT_data = fopen("output", "wb+");
	
	fprintf(result, "%s", PNG_SIGNATURE);
	
	fprintf(result, "%s", IHDR_LENGTH);
	fprintf(result, "%s", IHDR_CHUNK_TYPE);
	fprintf(result, "%08x", width);
	fprintf(result, "%08x", height);
	fprintf(result, "%s", IHDR_CHUNK_DATA);
	fprintf_checksum(12, 17, result);
	
	char my_char;
	while((my_char = fgetc(IDAT_data)) != EOF){
		fputs(my_char, result);
	}
	fprintf(result, "%08x", IDAT_length);
	fprintf(result, "%s", IDAT_CHUNK_TYPE);
	
	fprintf_checksum(37, 4 + IDAT_length, result);
	
	fprintf(result, "%s", IEND);
	fclose(result);
	return 0;
}

int main(int argc, char* argv[]){
	int image_count = argc - 1;
	int status = 0;
	
	int all_width = get_png_width(get_IHDR_data(argv[1])); /* verify if all images have same width */
	for(int i = 2; i <= image_count; ++i){
		if(all_width != get_png_width(get_IHDR_data(argv[i]))){
			return -1;
		}
	}
	
	int all_height = 0; /* calculates all.png height */
	for(int i = 1; i <= image_count; ++i){
		all_height += get_png_height(get_IHDR_data(argv[1]));
	}
	
	int IDAT_chunk_length = cat_raw_data(image_count, argv);
	
	generate_output(all_width, all_height, IDAT_chunk_length);
	
	return 0;
}