#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../stb_image/stb_image_write.h"

// Función para aplicar el filtro de desenfoque gaussiano a una imagen
void gaussianBlur(unsigned char *image, int width, int height, int channels, float sigma) {
    unsigned char *tempImage = (unsigned char *)malloc(width * height * channels * sizeof(unsigned char));
    if (tempImage == NULL) {
        fprintf(stderr, "Error al asignar memoria para la imagen temporal.\n");
        return;
    }
    for (int i = 0; i < width * height * channels; ++i) {
        tempImage[i] = image[i];
    }

    int radius = (int)(sigma * 3);
    int size = 2 * radius + 1;
    float *kernel = (float *)malloc(size * size * sizeof(float));
    if (kernel == NULL) {
        fprintf(stderr, "Error al asignar memoria para el kernel.\n");
        free(tempImage);
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
            kernel[i * size + j] /= sum;
        }
    }

    for (int y = radius; y < height - radius; ++y) {
        for (int x = radius; x < width - radius; ++x) {
            for (int c = 0; c < channels; ++c) {
                float newValue = 0.0f;
                for (int i = -radius; i <= radius; ++i) {
                    for (int j = -radius; j <= radius; ++j) {
                        newValue += tempImage[((y + i) * width + (x + j)) * channels + c] * kernel[(i + radius) * size + (j + radius)];
                    }
                }
                image[(y * width + x) * channels + c] = (unsigned char)newValue;
            }
        }
    }

    free(tempImage);
    free(kernel);
}

int main() {
    const char *imagePath = "image.jpg";
    int width, height, channels;
    unsigned char *image = stbi_load(imagePath, &width, &height, &channels, 0);
    if (!image) {
        fprintf(stderr, "Error al cargar la imagen.\n");
        return 1;
    }

    const float sigma = 5.5f;
    clock_t start = clock();
    gaussianBlur(image, width, height, channels, sigma);
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    if (!stbi_write_jpg("filtered_image.jpg", width, height, channels, image, 100)) {
        fprintf(stderr, "Error al guardar la imagen resultante.\n");
    } else {
        printf("Filtro de desenfoque gaussiano aplicado exitosamente.\n");
    }
    printf("Tiempo de ejecución: %.2f segundos\n", time_spent);

    stbi_image_free(image);

    return 0;
}

