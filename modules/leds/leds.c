#include "leds.h"

uint LEDs[2] = {LED_B, LED_R};

// Inicializa botões
void init_leds()
{
    for (int i = 0; i < 2; i++)
    {
        gpio_init(LEDs[i]);
        gpio_set_dir(LEDs[i], GPIO_OUT);
        gpio_put(LEDs[i], 0);
    }
}