#include "joystick.h"

const uint pins_joystick[3] = {JOYSTICK_X_PIN, JOYSTICK_Y_PIN};

void init_joystick()
{
    for (int i = 0; i < 3; i++)
    {
        adc_gpio_init(pins_joystick[i]);
    }

    // inicializando o botão
    gpio_init(BUTTON_JOYSTICK);
    gpio_pull_up(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
}

void reading_joystick()
{

    while (1)
    {
        adc_select_input(0);
        uint adc_y_raw = adc_read();
        adc_select_input(1);
        uint adc_x_raw = adc_read();
    }
}