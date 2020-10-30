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
int squeeze(const char* dest, uint8_t* buffer);