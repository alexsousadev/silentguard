#include "buttons.h"

int buttons[2] = {BUTTON_A, BUTTON_B};

void buttons_init()
{
    for (int i = 0; i < 2; i++)
    {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }
}
