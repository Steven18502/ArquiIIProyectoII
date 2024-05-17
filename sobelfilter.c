#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define IMG_W 6
#define IMG_L 6

float sobelPixel(unsigned char  matriz[3][3]){
    int sobelX[3][3] = {{-1,0,1},
                        {-2,0,2},
                        {-1,0,1}};

    int sobelY[3][3] = {{-1,-2,-1},
                        {0 ,0, 0},
                        {1, 2, 1}};
    float resX = 0;
    float resY = 0;
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            resX = matriz[j][i] * sobelX[j][i] + resX;
            resY = matriz[j][i] * sobelY[j][i] + resY;
        }
    }
    float pixelResult = sqrt(pow(resX,2)+pow(resY,2));
}

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


void genNeighMatrix(unsigned char* inArray,unsigned char neighMatrix[3][3],int j, int i,int height, int width){

    int row = j - 1;
    int col = i - 1; 
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if ((col >= 0 && col < width ) && (row < height && row >= 0)) {
                neighMatrix[y][x] = inArray[row * width + col];
            }
            col += 1;
        }
        col = i - 1;
        row += 1; 
    }
}
 
void sobelfilter(unsigned char* inArray, unsigned char* filterArray,int height ,int width){
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++)
        {   
            unsigned char neighMatrix[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
            genNeighMatrix(inArray, neighMatrix, j, i,height,width);
            float result = sobelPixel(neighMatrix);
            filterArray[j * width + i] = result;
        }
    }
}

int main(){

    int width, height, channels;
    unsigned char* image = stbi_load("/home/steven/Documents/Arqui2/miamor.jpeg", &width, &height, &channels, 0);
    if (image == NULL) {
        fprintf(stderr, "No se pudo cargar la imagen\n");
        return -1;
    }

    unsigned char* gray_image = convert_to_grayscale(image, width, height, channels);
    if (gray_image == NULL) {
        stbi_image_free(image);
        return -1;
    }    


    unsigned char* filter_image = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    sobelfilter(gray_image, filter_image, height, width);

    // Save filter image
    if (!stbi_write_png("/home/steven/Documents/Arqui2/sobel_image.png", width, height, 1, filter_image, width)) {
        fprintf(stderr, "No se pudo guardar la imagen en escala de grises\n");
        free(gray_image);
        free(filter_image);
        stbi_image_free(image);
        return -1;
    }

    // Liberar la memoria
    free(gray_image);
    free(filter_image);
    stbi_image_free(image);

    printf("Imagen en escala de grises guardada exitosamente\n");

    return 0;
}