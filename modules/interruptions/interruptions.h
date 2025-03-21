#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "modules/matriz_leds/matriz_leds.h"
#include "modules/buzzer/buzzer.h"
#include "modules/display/display.h"
#include "modules/joystick/joystick.h"

extern volatile bool menu_requested;

// Prototipos das funções
void init_interruptions(void);
void joystick_callback(uint gpio, uint32_t events);