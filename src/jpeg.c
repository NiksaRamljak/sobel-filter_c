#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"

#ifdef WITH_JPEG

#include <turbojpeg.h>

Image read_jpeg(const char *filename) {
    tjhandle handle = tjInitDecompress();
    if (!handle) {
        fprintf(stderr, "tjInitDecompress failed\n");
        exit(EXIT_FAILURE);
    }

    FILE *jpegFile = fopen(filename, "rb");
    if (!jpegFile) {
        perror("Unable to open JPEG file");
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    fseek(jpegFile, 0, SEEK_END);
    long jpegSize = ftell(jpegFile);
    fseek(jpegFile, 0, SEEK_SET);

    if (jpegSize <= 0) {
        fprintf(stderr, "Invalid JPEG file size.\n");
        fclose(jpegFile);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    unsigned char *jpegBuf = malloc(jpegSize);
    if (!jpegBuf) {
        perror("Memory allocation failed");
        fclose(jpegFile);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    size_t read_bytes = fread(jpegBuf, 1, jpegSize, jpegFile);
    fclose(jpegFile);
    if (read_bytes != jpegSize) {
        fprintf(stderr, "Incomplete JPEG file read.\n");
        free(jpegBuf);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    int width, height, jpegSubsamp, jpegColorspace;
    if (tjDecompressHeader3(handle, jpegBuf, jpegSize, &width, &height, &jpegSubsamp, &jpegColorspace) != 0) {
        fprintf(stderr, "tjDecompressHeader3 failed: %s\n", tjGetErrorStr());
        free(jpegBuf);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    if (width <= 0 || height <= 0 || width > 10000 || height > 10000) {
        fprintf(stderr, "Invalid image dimensions: %dx%d\n", width, height);
        free(jpegBuf);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    unsigned char *imgBuf = malloc(width * height);
    if (!imgBuf) {
        perror("Memory allocation failed");
        free(jpegBuf);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    if (tjDecompress2(handle, jpegBuf, jpegSize, imgBuf, width, width, height, TJPF_GRAY, TJFLAG_FASTDCT) != 0) {
        fprintf(stderr, "tjDecompress2 failed: %s\n", tjGetErrorStr());
        free(jpegBuf);
        free(imgBuf);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    tjDestroy(handle);
    free(jpegBuf);

    Image img = { width, height, imgBuf };
    return img;
}

void write_jpeg(const char *filename, int width, int height, unsigned char *data, int quality) {
    tjhandle handle = tjInitCompress();
    if (!handle) {
        fprintf(stderr, "tjInitCompress failed\n");
        exit(EXIT_FAILURE);
    }

    unsigned char *jpegBuf = NULL;
    unsigned long jpegSize = 0;

    if (tjCompress2(handle, data, width, 0, height, TJPF_GRAY, &jpegBuf, &jpegSize, TJSAMP_GRAY, quality, TJFLAG_FASTDCT) != 0) {
        fprintf(stderr, "tjCompress2 failed: %s\n", tjGetErrorStr());
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Unable to open file for writing JPEG");
        tjFree(jpegBuf);
        tjDestroy(handle);
        exit(EXIT_FAILURE);
    }

    fwrite(jpegBuf, 1, jpegSize, file);
    fclose(file);

    tjFree(jpegBuf);
    tjDestroy(handle);
}

#else

Image read_jpeg(const char *filename) {
    (void)filename;
    fprintf(stderr, "JPEG support not enabled. Please compile with WITH_JPEG=1.\n");
    exit(EXIT_FAILURE);
}

void write_jpeg(const char *filename, int width, int height, unsigned char *data, int quality) {
    (void)filename; (void)width; (void)height; (void)data; (void)quality;
    fprintf(stderr, "JPEG support not enabled. Please compile with WITH_JPEG=1.\n");
    exit(EXIT_FAILURE);
}

#endif