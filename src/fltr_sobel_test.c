#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <math.h>
#include "image.h"
#include "jpeg.h"
#include "pgm.h"
#include "thread_auto.h"

Image read_image(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext && strcmp(ext, ".pgm") == 0) {
        return read_pgm(filename);
    } else {
        return read_jpeg(filename);
    }
}

void write_image(const char *filename, int width, int height, unsigned char *data) {
    const char *ext = strrchr(filename, '.');
    if (ext && strcmp(ext, ".pgm") == 0) {
        write_pgm(filename, width, height, data);
    } else {
        // Default quality 90
        write_jpeg(filename, width, height, data, 90);
    }
}


// Apply Sobel ops
void apply_sobel(Image img, unsigned char *output, int start_row, int end_row) {
    int Gx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    int Gy[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

    int width = img.width;

    for (int row = start_row; row < end_row; ++row) {
        if (row == 0 || row == img.height - 1) continue;    //row skip
        for (int col = 1; col < width - 1; ++col) {
            int sum_x = 0, sum_y = 0;

            // Apply Sobel op on Gx and Gy (3x3 around)
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int pixel = img.data[(row + i) * width + (col + j)];
                    sum_x += pixel * Gx[i + 1][j + 1];
                    sum_y += pixel * Gy[i + 1][j + 1];
                }
            }

            // Calculate the magnitude
            int magnitude = (int)sqrt(sum_x * sum_x + sum_y * sum_y);
            // Hammer into 0-255 scale
            output[row * width + col] = (magnitude > 255) ? 255 : magnitude;
        }
    }
}

// Threading sobel
typedef struct {
    Image img;
    unsigned char *output;
    int start_row;
    int end_row;
} SobelArgs;

int sobel_thread_func(void *args) {
    SobelArgs *sobel_args = (SobelArgs *)args;
    apply_sobel(sobel_args->img, sobel_args->output, sobel_args->start_row, sobel_args->end_row);
    return 0;
}

// Parallel sobel
void apply_sobel_parallel(Image img, unsigned char *output, int num_threads) {
    thrd_t *threads = (thrd_t *)malloc(num_threads * sizeof(thrd_t));
    SobelArgs *args = (SobelArgs *)malloc(num_threads * sizeof(SobelArgs));

    int rows_per_thread = img.height / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        args[i].img = img;
        args[i].output = output;
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i == num_threads - 1) ? img.height : (i + 1) * rows_per_thread;

        thrd_create(&threads[i], sobel_thread_func, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        thrd_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}


int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <input_image.{pgm|jpg}> <output_image.{pgm|jpg}> [num_threads]\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    int user_override = 0;
    if (argc == 4) {
        user_override = atoi(argv[3]);
        if (user_override <= 0) {
            fprintf(stderr, "Invalid number of threads specified. Must be a positive integer.\n");
            return 1;
        }
    }

    Image img = read_image(input_filename);

    unsigned char *output = malloc(img.width * img.height);
    if (!output) {
        perror("Memory allocation failed");
        free(img.data);
        exit(1);
    }

    int num_threads = calc_thread(img, user_override);
    printf("Using %d thread%s\n", num_threads, num_threads > 1 ? "s" : "");

    apply_sobel_parallel(img, output, num_threads);

    write_image(output_filename, img.width, img.height, output);

    free(img.data);
    free(output);

    return 0;
}
