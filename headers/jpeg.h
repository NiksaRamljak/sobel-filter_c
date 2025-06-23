#ifndef JPEG_H
#define JPEG_H

#include "image.h"

Image read_jpeg(const char *filename);
void write_jpeg(const char *filename, int width, int height, unsigned char *data, int quality);

#endif