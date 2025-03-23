#include "pico/stdlib.h"
#include "modules/display/menu/menu.h"
#include "modules/joystick/joystick.h"

extern volatile bool menu_requested;

// Prototipos das funções
void init_interruptions(void);
void joystick_callback(uint gpio, uint32_t events);