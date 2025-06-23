#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    int width, height;
    unsigned char *data;
} Image;
#endif