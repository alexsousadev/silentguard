
#include "buzzer.h"

#define BUZZER 21
#define LOOP_BEEP 3

// Inialização do buzzer
void init_buzzer()
{
    // Configuração do PWM
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER, 0); // desliga pwm
}

// Ativa o Buzzer em uma frequência e duração determinada
void beepBuzzer(uint frequency, uint duration)
{
    pwm_set_gpio_level(BUZZER, frequency);
    sleep_ms(duration);
    pwm_set_gpio_level(BUZZER, 0);
    sleep_ms(duration);
}