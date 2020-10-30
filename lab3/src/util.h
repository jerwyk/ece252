#pragma once

#include "png.h"
#include "crc.h"
#include "zutil.h"

/**
* @brief concentrate mul
* @param dest the concatenated image file path
* @param pngs the png images to be concatenated
* @param num the number of images
*/
int catpng(const char* dest, char **pngs, size_t num);