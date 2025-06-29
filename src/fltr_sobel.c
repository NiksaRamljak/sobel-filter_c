#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <threads.h>

//Imageframe
typedef struct {
    int width, height;
    unsigned char *data;
} Image;

// Image  loader
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

// Free image memory
void free_image(Image img) {
    free(img.data);
}

// Apply Sobel ops
void apply_sobel(Image img, unsigned char *output, int start_row, int end_row) {
    int Gx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    int Gy[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

    int width = img.width;

    for (int row = start_row; row < end_row; ++row) {
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

// threads based on image size
int calc_thread(Image img, int user_override) {
    int total_size = img.width * img.height;
    // rough optimization
    if (user_override>0) {
        return user_override;
    }else if (total_size< 500* 1024) {			//5k
        return 1; 
    } else if (total_size < 1000 * 1024) {		//1m
        return 2;
    } else if (total_size < 4000 * 1024) {	    //4m
        return 4;
    } else {
        return 8;
    }
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

// Write back to PGM
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

int main(int argc, char *argv[]) {
    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    int user_override = 0;

    // Accept only if "-t <num_threads>"
    if (argc == 5 && strcmp(argv[3], "-t") == 0) {
        user_override = atoi(argv[4]);
        if (user_override <= 0) {
            fprintf(stderr, "Invalid number of threads specified. Must be a positive integer.\n");
            return 1;
        }
    } else if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_image.{pgm|jpg}> <output_image.{pgm|jpg}> [-t num_threads]\n", argv[0]);
        return 1;
    }

    // Read in image
    Image img = read_pgm(input_filename);

    // Allocate memory for  out
    unsigned char *output = (unsigned char *)malloc(img.width * img.height);

    //scaling
    int num_threads = calc_thread(img, user_override);
    printf("Using %d threads\n", num_threads);

    apply_sobel_parallel(img, output, num_threads);

    write_pgm(output_filename, img.width, img.height, output);

    free_image(img);
    free(output);

    return 0;
}
