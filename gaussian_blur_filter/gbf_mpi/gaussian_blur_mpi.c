#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../stb_image/stb_image_write.h"

// Function to apply Gaussian blur on a segment of the image, considering overlap
void gaussianBlur(unsigned char *image, int width, int rows, int channels, float sigma, int overlap) {
    int radius = (int)(sigma * 3);
    int size = 2 * radius + 1;
    float *kernel = (float *)malloc(size * size * sizeof(float));
    if (!kernel) {
        fprintf(stderr, "Error allocating memory for the kernel.\n");
        return;
    }

    float sum = 0.0f;
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            float x = i * i + j * j;
            kernel[(i + radius) * size + (j + radius)] = exp(-x / (2 * sigma * sigma));
            sum += kernel[(i + radius) * size + (j + radius)];
        }
    }
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            kernel[i * size + j] /= sum;  // Ensure 'j' is declared within its loop
        }
    }

    unsigned char *tempImage = (unsigned char *)malloc(width * rows * channels * sizeof(unsigned char));
    if (!tempImage) {
        fprintf(stderr, "Error allocating memory for the temporary image.\n");
        free(kernel);
        return;
    }
    memcpy(tempImage, image, width * rows * channels);

    // Apply the Gaussian blur on the actual image data, excluding the overlap regions for output
    for (int y = overlap; y < rows - overlap; ++y) {
        for (int x = radius; x < width - radius; ++x) {
            for (int c = 0; c < channels; ++c) {
                float newValue = 0.0f;
                for (int i = -radius; i <= radius; ++i) {
                    for (int j = -radius; j <= radius; ++j) {
                        int img_y = y + i;
                        int img_x = x + j;
                        if (img_y >= overlap && img_y < rows - overlap) {
                            newValue += tempImage[(img_y * width + img_x) * channels + c] * kernel[(i + radius) * size + (j + radius)];
                        }
                    }
                }
                image[(y * width + x) * channels + c] = (unsigned char)fminf(fmaxf(newValue, 0.0f), 255.0f);
            }
        }
    }

    free(tempImage);
    free(kernel);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int width, height, channels;
    unsigned char *image = NULL;
    double start_time, end_time;

    if (world_rank == 0) {
        start_time = MPI_Wtime();
        image = stbi_load("image.jpg", &width, &height, &channels, 0);
        if (!image) {
            fprintf(stderr, "Error loading the image.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_per_proc = height / world_size;
    int extra_rows = height % world_size;
    int start_row = world_rank * rows_per_proc + (world_rank < extra_rows ? world_rank : extra_rows);
    int end_row = start_row + rows_per_proc + (world_rank < extra_rows ? 1 : 0);
    int local_rows = end_row - start_row;

    unsigned char *local_image = (unsigned char *)malloc(local_rows * width * channels * sizeof(unsigned char));
    if (local_image == NULL) {
        fprintf(stderr, "Error allocating memory for the local image.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(image, local_rows * width * channels, MPI_UNSIGNED_CHAR, local_image, local_rows * width * channels, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    gaussianBlur(local_image, width, local_rows, channels, 5.5f, 1);

    MPI_Gather(local_image + width * channels, (local_rows - 2) * width * channels, MPI_UNSIGNED_CHAR, image + start_row * width * channels + width * channels, (local_rows - 2) * width * channels, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        if (!stbi_write_jpg("filtered_image.jpg", width, height, channels, image, 100)) {
            fprintf(stderr, "Failed to save the output image\n");
        } else {
            printf("Gaussian Blur filtered image saved successfully\n");
        }

        stbi_image_free(image);
        end_time = MPI_Wtime();
        printf("Execution time: %f seconds\n", end_time - start_time);
    }

    free(local_image);

    MPI_Finalize();
    return 0;
}

