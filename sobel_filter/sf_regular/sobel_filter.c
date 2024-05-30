#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../stb_image/stb_image_write.h"

// Calculate the Sobel filter on a single pixel
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

// Convert an RGB image to grayscale
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

// Apply the Sobel filter to an image
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

int main() {
    int width, height, channels;
    unsigned char* image = stbi_load("image.jpg", &width, &height, &channels, 0);
    if (!image) {
        fprintf(stderr, "Failed to load the image\n");
        return -1;
    }

    unsigned char* gray_image = convert_to_grayscale(image, width, height, channels);
    if (!gray_image) {
        stbi_image_free(image);
        return -1;
    }

    unsigned char* output_image = malloc(width * height);
    if (!output_image) {
        fprintf(stderr, "Failed to allocate memory for output image\n");
        free(gray_image);
        stbi_image_free(image);
        return -1;
    }

    clock_t start_time = clock();
    apply_sobel_filter(gray_image, output_image, width, height);
    clock_t end_time = clock();

    if (!stbi_write_png("filtered_image.png", width, height, 1, output_image, width)) {
        fprintf(stderr, "Failed to save the output image\n");
    } else {
        printf("Sobel filtered image saved successfully\n");
    }

    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution Time: %.2f seconds\n", time_taken);

    free(output_image);
    free(gray_image);
    stbi_image_free(image);
    return 0;
}

