#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../stb_image/stb_image_write.h"

float sobelPixel(unsigned char matrix[3][3]) {
    int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    float gx = 0, gy = 0;
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            gx += matrix[y][x] * sobelX[y][x];
            gy += matrix[y][x] * sobelY[y][x];
        }
    }
    return sqrt(gx * gx + gy * gy);
}

unsigned char* convert_to_grayscale(unsigned char* image, int width, int height, int channels) {
    unsigned char* gray_image = malloc(width * height);
    if (!gray_image) {
        fprintf(stderr, "Failed to allocate memory for grayscale image\n");
        return NULL;
    }
    for (int i = 0; i < width * height; i++) {
        int r = image[i * channels];
        int g = image[i * channels + 1];
        int b = image[i * channels + 2];
        gray_image[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }
    return gray_image;
}

void apply_sobel_filter(unsigned char* gray_image, unsigned char* output_image, int width, int height) {
    for (int j = 1; j < height - 1; j++) {
        for (int i = 1; i < width - 1; i++) {
            unsigned char window[3][3];
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    window[y + 1][x + 1] = gray_image[(j + y) * width + (i + x)];
                }
            }
            float pixel_value = sobelPixel(window);
            output_image[j * width + i] = (unsigned char)(pixel_value > 255 ? 255 : pixel_value);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int width, height, channels;
    unsigned char *image = NULL, *gray_image = NULL, *output_image = NULL;
    double start_time, end_time;

    if (world_rank == 0) {
        image = stbi_load("image.jpg", &width, &height, &channels, 0);
        if (!image) {
            fprintf(stderr, "Failed to load the image\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        gray_image = convert_to_grayscale(image, width, height, channels);
        output_image = malloc(width * height);
        start_time = MPI_Wtime();
    }

    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_height = height / world_size;
    int start_row = world_rank * local_height;
    int end_row = (world_rank == world_size - 1) ? height : (world_rank + 1) * local_height;

    unsigned char* local_gray = malloc(local_height * width);
    unsigned char* local_output = malloc(local_height * width);

    MPI_Scatter(gray_image, local_height * width, MPI_UNSIGNED_CHAR, local_gray, local_height * width, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    apply_sobel_filter(local_gray, local_output, width, local_height);

    MPI_Gather(local_output, local_height * width, MPI_UNSIGNED_CHAR, output_image, local_height * width, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        if (!stbi_write_png("filtered_image.png", width, height, 1, output_image, width)) {
            fprintf(stderr, "Failed to save the output image\n");
        } else {
            printf("Sobel filtered image saved successfully\n");
        }
        end_time = MPI_Wtime();
        printf("Execution Time: %f seconds\n", end_time - start_time);

        free(image);
        free(gray_image);
        free(output_image);
    }

    free(local_gray);
    free(local_output);

    MPI_Finalize();
    return 0;
}

