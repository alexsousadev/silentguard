#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#define LED_COUNT 25
#define LED_PIN 7

// Protótipos das funções
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
void matrizToLeds(int draw[5][5], int color[3]);
void convertMatrizToLed(int pixelDraw[5][5], int pixelRGB[5][5][3]);
void init_matriz_leds();
void emojiFeliz();
void emojiTriste();
void convertPiskelToMatriz(uint32_t piskel_data[1][25], int pixelDraw[5][5], int pixelRGB[5][5][3]);