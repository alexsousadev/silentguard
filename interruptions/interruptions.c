#include "interruptions.h"
#include "joystick/joystick.h"

// Inicialização das Interrupções
void init_interruptions()
{
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_RISE, true, &joystick_callback);
}

// Callback do botão de configuração
void joystick_callback(uint gpio, uint32_t events)
{
    menu_config();
}