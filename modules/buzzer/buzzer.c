
#include "buzzer.h"
#include "modules/core/pwm.h"

#define BUZZER 21
#define LOOP_BEEP 3

// Inialização do buzzer
void init_buzzer()
{
    pwm_on(BUZZER);
}

// Ativa o Buzzer em uma frequência e duração determinada
void beep_buzzer(uint frequency, uint duration)
{
    pwm_set_level(BUZZER, frequency);
    sleep_ms(duration);
    pwm_set_level(BUZZER, 0);
    sleep_ms(duration);
}