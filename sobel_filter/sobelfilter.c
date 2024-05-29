#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
//#include <thread>
#include <math.h>
#define IMG_W 6
#define IMG_L 6


// Realiza el calculo del filtro de sobel sobre un pixel
// Inputs: matriz 3x3 con el pixel y sus vecinos
// Output: float con el resultado del valor del pixel en la imagen de salida 
float sobelPixel(unsigned char  matriz[3][3]){
    // Mascaras para el filtro en X y en Y
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

// Función para convertir una imagen RGB a escala de grises para aplicar el filtro
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


// Funcion para dividir una imagen en N
// Entradas puntero a la imagen, altura, ancho. puntero al array donde guardar los punteros a las nuevas imagens, puntero al array donde guardar las alturas, numero a dividir
void divideArray(unsigned char* originalArray, int height, int width, unsigned char** subArrays, int* heights, int N) {    
    // Calculate the size of each sub-array
    int baseHeight = height / N;
    int remainder = height % N;

    // Allocate memory for each sub-array and copy data
    int offset = 0;
    for (int i = 0; i < N; ++i) {
        int subArrayHeight = baseHeight + (i < remainder ? 1 : 0);
        heights[i] = subArrayHeight;
        subArrays[i] = (unsigned char*)malloc(subArrayHeight * width * sizeof(unsigned char));
        memcpy(subArrays[i], originalArray + offset, subArrayHeight * width * sizeof(unsigned char));
        offset += subArrayHeight * width;
    }
}


// Funcion para generar la matriz 3x3 con el pixel a procesar y sus vecinos 
// Inputs: puntero al array con la imagen a procesar, matriz 3x3 con ceros donde almacenar el pixel y los vecinos, 
// numero de fila del pixel,numero de columna del pixel,altura de la imagen, ancho de la imagen

void genNeighMatrix(unsigned char* inArray,unsigned char neighMatrix[3][3],int j, int i,int height, int width){
    // Posiciones del primer pixel vecino de la matriz 
    int row = j - 1;
    int col = i - 1; 
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            // Si las posciones existen, para los casos de los bordes
            if ((col >= 0 && col < width ) && (row < height && row >= 0)) {
                neighMatrix[y][x] = inArray[row * width + col];
            }
            col += 1;
        }
        col = i - 1;
        row += 1; 
    }
}

// Funcion para aplicar el filtro de sobel sobre una imagen 
// Inputs: puntero a la imagen a procesar, puntero al array donde se almacena el resultado, altura y ancho de la imagen, numero del bloque de la imagen que se esta procesando
void sobelfilter(unsigned char* inArray,unsigned char* filterArray,int height ,int width, int block_num){
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++)
        {   
            unsigned char neighMatrix[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
            genNeighMatrix(inArray, neighMatrix, j, i,height,width); // Genera la matriz para el pixel actual y los vecinos
            float result = sobelPixel(neighMatrix); // Calcula el resultado para el pixel actual 
            filterArray[(j+block_num) * width + i] = result; // Guarda el valor calculado para e pixel actual de la imagen filtrada
        }
    }
}

int main(){
    // Se carga la imagen en un array
    int width, height, channels;
    unsigned char* image = stbi_load("/home/steven/Documents/Arqui2/Proyecto2/image.jpg", &width, &height, &channels, 0);
    if (image == NULL) {
        fprintf(stderr, "No se pudo cargar la imagen\n");
        return -1;
    }

    int thread_num = 4;

    // Array para los punteros a los sub arrays de la imagen
    unsigned char** subArrays = (unsigned char**)malloc(4 * sizeof(int));
    // Array con el tamaño en filas de cada sub array
    int* heights = (int*)malloc(4 * sizeof(int));

    

     // Se convierte a escala de grises y se guarda en otro array
    unsigned char* gray_image = convert_to_grayscale(image, width, height, channels);
    if (gray_image == NULL) {
        stbi_image_free(image);
        return -1;
    }    

    // Se divide la imagen segun el numero de threads
    divideArray(gray_image,height,width,subArrays,heights,thread_num);

    // Se guarda el espacio de memoria para alamcenar la imagen filtrada
    unsigned char* filter_image = (unsigned char*)malloc(width * height * sizeof(unsigned char));

    // Se llama varias veces a la funcion principal con cada sub array 
    sobelfilter(subArrays[0],filter_image,heights[0],width,0);
    sobelfilter(subArrays[1],filter_image,heights[1],width,heights[0]);
    sobelfilter(subArrays[2],filter_image,heights[2],width,heights[0]+heights[1]);
    sobelfilter(subArrays[3],filter_image,heights[3],width,heights[0]+heights[1]+heights[2]);


    // Se crea la nueva imagen y se libera la memoria 
    if (!stbi_write_png("/home/steven/Documents/Arqui2/Proyecto2/sobel_image.png", width, height, 1, filter_image, width)) {
        fprintf(stderr, "No se pudo guardar la imagen en escala de grises\n");
        return -1;
    }

    // Liberar la memoria
    free(gray_image);
    free(filter_image);
    stbi_image_free(image);
    // Free the allocated memory
    for (int i = 0; i < thread_num; ++i) {
        free(subArrays[i]);
    }
    free(heights);
    free(subArrays);
    printf("Imagen filtrada guardada exitosamente\n");

    return 0;
}