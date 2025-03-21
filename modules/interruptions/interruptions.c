#include "interruptions.h"
#include "modules/joystick/joystick.h"
#include "modules/matriz_leds/matriz_leds.h"

volatile bool menu_requested = false;

// Inicialização das Interrupções
void init_interruptions()
{
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &joystick_callback);
}

// Callback do botão de configuração
void joystick_callback(uint gpio, uint32_t events)
{
    all_options_config();
}