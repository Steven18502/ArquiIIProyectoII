#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Función para aplicar el filtro de desenfoque gaussiano a una imagen
void gaussianBlur(unsigned char *image, int width, int height, int channels, float sigma) {
    // Copiar la imagen original
    unsigned char *tempImage = (unsigned char *)malloc(width * height * channels * sizeof(unsigned char)); // Reserva memoria para la imagen temporal
    if (tempImage == NULL) {
        fprintf(stderr, "Error al asignar memoria para la imagen temporal.\n"); // Maneja el caso de falta de memoria
        return;
    }
    for (int i = 0; i < width * height * channels; ++i) {
        tempImage[i] = image[i]; // Copiar la imagen original en la imagen temporal
    }

    // Calcula el tamaño de la máscara
    int radius = (int)(sigma * 3); // Calcula el radio de la máscara según el parámetro sigma

    // Calcula la máscara gaussiana
    int size = 2 * radius + 1; // Calcula el tamaño de la máscara
    float *kernel = (float *)malloc(size * size * sizeof(float)); // Reserva memoria para el kernel
    if (kernel == NULL) {
        fprintf(stderr, "Error al asignar memoria para el kernel.\n"); // Maneja el caso de falta de memoria
        free(tempImage);
        return;
    }
    float sum = 0.0f; // Inicializa la suma de valores del kernel
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            float x = i * i + j * j;
            kernel[(i + radius) * size + (j + radius)] = exp(-x / (2 * sigma * sigma)); // Calcula el valor de la máscara gaussiana
            sum += kernel[(i + radius) * size + (j + radius)]; // Calcula la suma total de los valores del kernel
        }
    }

    // Normaliza la máscara
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            kernel[i * size + j] /= sum; // Normaliza los valores del kernel dividiendo por la suma total
        }
    }

    // Aplicando el filtro
    for (int y = radius; y < height - radius; ++y) {
        for (int x = radius; x < width - radius; ++x) {
            for (int c = 0; c < channels; ++c) {
                float newValue = 0.0f; // Inicializa el nuevo valor de píxel
                for (int i = -radius; i <= radius; ++i) {
                    for (int j = -radius; j <= radius; ++j) {
                        newValue += tempImage[((y + i) * width + (x + j)) * channels + c] * kernel[(i + radius) * size + (j + radius)]; // Aplica el filtro de convolución
                    }
                }
                image[(y * width + x) * channels + c] = (unsigned char)newValue; // Actualiza el valor del píxel en la imagen original
            }
        }
    }

    // Liberar memoria
    free(tempImage);
    free(kernel);
}

int main() {
    // Ruta de la imagen a cargar
    const char *imagePath = "./image.jpg";

    // Cargar la imagen
    int width, height, channels;
    unsigned char *image = stbi_load(imagePath, &width, &height, &channels, 0);
    if (!image) {
        fprintf(stderr, "Error al cargar la imagen.\n");
        return 1;
    }

    // Parámetros del filtro de desenfoque gaussiano
    const float sigma = 5.5f; // Parámetro sigma para el filtro de desenfoque gaussiano

    // Aplicar el filtro de desenfoque gaussiano
    gaussianBlur(image, width, height, channels, sigma);

    // Guardar la imagen resultante
    if (!stbi_write_jpg("imageout.jpg", width, height, channels, image, 100)) {
        fprintf(stderr, "Error al guardar la imagen resultante.\n");
    }

    // Liberar memoria
    stbi_image_free(image);

    printf("Filtro de desenfoque gaussiano aplicado exitosamente.\n");

    return 0;
}
