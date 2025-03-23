#include "modules/core/pwm.h"

// Inicialização do PWM
void pwm_on(uint pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, true);
}

// Definindo o duty cycle do PWM
void pwm_set_level(uint pin, uint level)
{
    pwm_set_gpio_level(pin, level);
}