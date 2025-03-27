#include "pico/stdlib.h"
#include "sound_monitor.h"

//==============================================================================
// Função Principal
//==============================================================================
int main()
{
    stdio_init_all();
    init_all_gpios();
    init_interruptions();

    while (true)
    {
        float db_value = mic_get_db();
        define_action(db_value); // Ações baseadas no nível de dB
        sleep_ms(100);
    }
}