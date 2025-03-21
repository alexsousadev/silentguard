#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// Protótipos de funções
void init_buzzer(void);
void beepBuzzer(uint frequency, uint duration);
