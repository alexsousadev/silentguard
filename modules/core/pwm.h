#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Prototipos das funções
void pwm_on(uint pin);
void pwm_set_level(uint pin, uint level);