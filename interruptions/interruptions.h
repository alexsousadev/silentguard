#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "matriz_leds/matriz_leds.h"
#include "buzzer/buzzer.h";
#include "display/display.h";
#include "joystick/joystick.h";

// Prototipos das funções
void init_interruptions(void);
void joystick_callback(uint gpio, uint32_t events);