#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "modules/matriz_leds/matriz_leds.h"
#include "modules/display/display.h"
#include <math.h>

#define BUTTON_JOYSTICK 22

// Protótipos de funções
void init_joystick();