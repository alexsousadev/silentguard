#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
// Inicialização do PWM
void pwm_on(uint pin);
void pwm_set_level(uint pin, uint level);