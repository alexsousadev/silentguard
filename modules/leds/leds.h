#include "pico/stdlib.h"
// Pinos dos botões A e B
#define LED_B 12
#define LED_R 13

extern uint LEDs[2];

// Protótipos de funções
void init_leds(void);