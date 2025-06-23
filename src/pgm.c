#include <stdio.h>
#include <stdlib.h>
#include "pgm.h"

// PGM loader
Image read_pgm(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open file");
        exit(1);
    }

    // Read PGM  type tag (P5)
    char header[3];
    fscanf(file, "%2s", header);
    if (header[0] != 'P' || header[1] != '5') {
        fprintf(stderr, "Invalid format\n");
        exit(1);
    }

    // Skip A0 delimiter
    fgetc(file);

    // Skip comment
    char ch;
    while ((ch = fgetc(file)) == '#') {
        while ((ch = fgetc(file)) != '\n' && ch != EOF);  // Skip comment 
    }
    ungetc(ch, file);  // Put the  char outside comment back

    // Read width, height, and max pixel value
    int width = 0, height = 0, max_val = 0;

    if (fscanf(file, " %d", &width) != 1 || fscanf(file, " %d", &height) != 1 || fscanf(file, " %d", &max_val) != 1) {
        fprintf(stderr, "Error: Failed to read image frame data\n");
        exit(1);
    }

    fgetc(file);  // Skip A0 after max value
    
    // Allocate memory for image data
    unsigned char *data = (unsigned char *)malloc(width * height);
    if (!data) {
        perror("Failed to allocate memory");
        exit(1);
    }

    // Read image data
    fread(data, sizeof(unsigned char), width * height, file);

    fclose(file);

    Image img = {width, height, data};
    return img;
}

// PGM writer
void write_pgm(const char *filename, int width, int height, unsigned char *data) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Unable to open file");
        exit(1);
    }

    fprintf(file, "P5\n#N-man and his machine forged this\n%d %d\n255\n", width, height);
    fwrite(data, sizeof(unsigned char), width * height, file);

    fclose(file);
}