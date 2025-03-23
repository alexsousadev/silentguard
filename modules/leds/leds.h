#include "pico/stdlib.h"

// Pinos dos LEDs
#define LED_B 12
#define LED_R 13

extern uint LEDs[2];

// Protótipos de funções
void init_leds(void);