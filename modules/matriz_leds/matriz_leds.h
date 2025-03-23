#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#define LED_COUNT 25
#define LED_PIN 7

// Protótipos das funções
void np_init(uint pin);
void np_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void np_clear();
void np_write();
void matriz_to_leds(int draw[5][5], int color[3]);
void convert_matriz_to_led(int pixel_draw[5][5], int pixel_rgb[5][5][3]);
void init_matriz_leds();
void happy_emoji();
void sad_emoji();
void convert_piskel_to_matriz(uint32_t piskel_data[1][25], int pixel_draw[5][5], int pixel_rgb[5][5][3]);