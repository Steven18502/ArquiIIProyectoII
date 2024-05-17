#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>

// Función para convertir una imagen RGB a escala de grises
unsigned char* convert_to_grayscale(unsigned char* image, int width, int height, int channels) {
    unsigned char* gray_image = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (gray_image == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        return NULL;
    }

    for (int i = 0; i < width * height; ++i) {
        int r = image[i * channels];
        int g = image[i * channels + 1];
        int b = image[i * channels + 2];
        gray_image[i] = (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);
    }

    return gray_image;
}

int main() {
    int width, height, channels;
    unsigned char* image = stbi_load("/home/steven/Documents/Arqui2/image.jpg", &width, &height, &channels, 0);
    if (image == NULL) {
        fprintf(stderr, "No se pudo cargar la imagen\n");
        return -1;
    }

    unsigned char* gray_image = convert_to_grayscale(image, width, height, channels);
    if (gray_image == NULL) {
        stbi_image_free(image);
        return -1;
    }



    // Guardar la imagen en escala de grises
    if (!stbi_write_png("/home/steven/Documents/Arqui2/grey_image.png", width, height, 1, gray_image, width)) {
        fprintf(stderr, "No se pudo guardar la imagen en escala de grises\n");
        free(gray_image);
        stbi_image_free(image);
        return -1;
    }

    // Liberar la memoria
    free(gray_image);
    stbi_image_free(image);

    printf("Imagen en escala de grises guardada exitosamente\n");

    return 0;
}
