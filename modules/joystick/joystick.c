#include "joystick.h"
#include "modules/mic/mic.h"

// Inicialização do Joystick
void init_joystick()
{
    gpio_init(BUTTON_JOYSTICK);
    gpio_pull_up(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
}