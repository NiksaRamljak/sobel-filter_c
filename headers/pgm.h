#ifndef PGM_H
#define PGM_H

#include "image.h"

Image read_pgm(const char *filename);
void write_pgm(const char *filename, int width, int height, unsigned char *data);

#endif