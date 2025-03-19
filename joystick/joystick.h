#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "matriz_leds/matriz_leds.h"
#include "display/display.h"
#include <math.h>

#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27

#define BUTTON_JOYSTICK 22

// Protótipos de funções
void init_joystick();
void reading_joystick();