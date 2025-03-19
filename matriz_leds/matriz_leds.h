#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Protótipos das funções

// Inicializa a máquina PIO para controlar a matriz de LEDs
void npInit(uint pin);

// Define a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);

// Limpa o buffer de pixels
void npClear();

// Escreve os dados do buffer nos LEDs
void npWrite();

// Converte uma matriz de desenho para a matriz de LEDs
void matrizToLeds(int draw[5][5], int color[3]);

// Converte a matriz de pixels RGB para a matriz de LEDs
void convertMatrizToLed(int pixelDraw[5][5], int pixelRGB[5][5][3]);

// Inicializa a matriz de LEDs
void init_matriz_leds();

// Exibe um emoji feliz na matriz de LEDs
void emojiFeliz();

// Exibe um emoji triste na matriz de LEDs
void emojiTriste();

// Função para converter os dados do Piskel para a matriz (precisa ser implementada)
void convertPiskelToMatriz(uint32_t piskel_data[1][25], int pixelDraw[5][5], int pixelRGB[5][5][3]);