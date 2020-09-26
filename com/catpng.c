#include "catpng.h"
#include "png.h"

data_IHDR_p* get_IHDR_data(char* directory){
	simple_PNG_p png_image;
	memset(png_image, 0, sizeof(simple_PNG_p));
	FILE* png_file = fopen(directory, "rb");
	read_simple_png(png_image, png_file);
	result = png_image->p_IHDR->p_data;
	fclose(png_file);
	return result;
}

uint32_t get_IDAT_length(char* directory){
	simple_PNG_p png_image;
	memset(png_image, 0, sizeof(simple_PNG_p));
	FILE* png_file = fopen(directory, "rb");
	read_simple_png(png_image, png_file);
	result = png_image->p_IDAT->length;
	fclose(png_file);
	return result;
}

uint8_t* get_IDAT_data(char* directory){
	simple_PNG_p png_image;
	memset(png_image, 0, sizeof(simple_PNG_p));
	FILE* png_file = fopen(directory, "rb");
	read_simple_png(png_image, png_file);
	result = png_image->p_IDAT->p_data;
	fclose(png_file);
	return result;
}

int cat_raw_data(int image_count, char* dirs[]){
	FILE* temp = fopen("temp", "wb+");
	int IDAT_length = 0;
	for(int i = 1; i <= image_count; ++i){
		decompress()
		write_to_output()
	}
	FILE* output = fopen("output", "wb+");
	recompress(temp -> output)
	fclose(temp);
	fclose(output);
	return IDAT_length;
}

void fprintf_checksum(int offset, int length, FILE* filename){
	void* buffer = malloc(length);
	fseek(filename, offset, SEEK_SET);
	fread(buffer, 1, length, filename);
	fprintf(filename, "%08x", crc(buffer, length));
}

int generate_output(int width, int height, int IDAT_length){
	FILE* result = fopen("all.png", "wb+");
	fprintf(result, "%s", PNG_SIGNATURE);
	
	fprintf(result, "%s", IHDR_LENGTH);
	fprintf(result, "%s", IHDR_CHUNK_TYPE);
	fprintf(result, "%08x", width);
	fprintf(result, "%08x", height);
	fprintf(result, "%s", IHDR_CHUNK_DATA)
	fprintf_checksum(12, 17, result);
	
	fprintf(result, "%08x", IDAT_length);
	fprintf(result, "%s", IDAT_CHUNK_TYPE);
	fprintf(result, "%s", "temp");
	fprintf_checksum(37, 4 + IDAT_length, result);
	
	fprintf(result, "%s", IEND);
	fclose(result);
	return 0;
}

int main(int argc, char* argv[]){
	int image_count = argc - 1;
	int status = 0;
	
	for(int i = 1; i <= image_count; ++i){ /* verify if all images are valid */
		status = findpng(argv[i]);
		if(status == -1){
			return status;
		}
	}
	
	int all_width = get_IHDR_data(argv[1])->width; /* verify if all images have same width */
	for(int i = 2; i <= image_count; ++i){
		if(all_width != get_IHDR_data(argv[1])->width){
			return -1;
		}
	}
	
	int all_height = 0; /* calculates all.png height */
	for(int i = 1; i <= image_count; ++i){
		all_height += get_IHDR_data(argv[1])->height;
	}
	
	int IDAT_chunk_length = cat_raw_data(image count, argv[]);
	
	generate_output(all_width, all_height, IDAT_chunk_length);
	
	return 0;
}