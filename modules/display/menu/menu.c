#include "modules/display/menu/menu.h"
#include "modules/mic/mic.h"
#include "modules/joystick/joystick.h"
#include "modules/display/display.h"

#define CALIBRATION_DURATION_MS 4000 // Tempo de calibração (ms)

// Cria um intervalo de espera
void create_interval(uint32_t interval, float sum_db, int sample_count)
{
    absolute_time_t next_wake_time = delayed_by_us(get_absolute_time(), interval * 1000);

    while (!time_reached(next_wake_time))
    {
    }
}

// Calibração automatica dos dB
void configDisplayAutomatic()
{
    float sum_db = 0;
    int sample_count = 0;
    // Exibe mensagem de calibração no display
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Calibrando...", 20, 20);
    ssd1306_send_data(&ssd);
    create_interval(CALIBRATION_DURATION_MS / 2, sum_db, sample_count);
    create_interval(CALIBRATION_DURATION_MS / 2, sum_db, sample_count);
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Quase la...", 20, 20);
    ssd1306_send_data(&ssd);
    create_interval(CALIBRATION_DURATION_MS, sum_db, sample_count);
    ssd1306_clear_screen(&ssd);
    float ambient_average = mic_get_db();
    valorDecibeis = ambient_average + 7;
    ssd1306_draw_string(&ssd, "Calibrado", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);
    ssd1306_draw_string(&ssd, "Novo lim.: ", 10, 30);
    char *newDb = intToString(ambient_average);
    ssd1306_draw_string(&ssd, newDb, 80, 30);
    ssd1306_draw_string(&ssd, " dB", 95, 30);
    free(newDb);
    ssd1306_draw_string(&ssd, "Ja pode sair", 20, 40);
    ssd1306_send_data(&ssd);
    npClear();
}

// Configuração manual dos dB
void configDisplayManual()
{
    menuActive = true;
    ssd1306_clear_screen(&ssd);
    drawMenuScreen();

    while (menuActive)
    {
        if (debounce(BUTTON_B)) // Botão B: aumenta dB
        {
            valorDecibeis += VALUE_INCREMENT_VOLUME;
            if (valorDecibeis >= 105)
            {
                valorDecibeis = MIN_VALUE_DB;
            }
            drawMenuScreen();
        }

        if (debounce(BUTTON_JOYSTICK)) // Botão A: sai do menu
        {
            menuActive = false;
        }
    }
    ssd1306_clear_screen(&ssd); // Limpa tela ao sair do menu
    ssd1306_send_data(&ssd);
}

// Seleciona uma opção do menu (manual ou automático)
void select_option_config(bool option)
{

    switch (option)
    {
    case false:
        configDisplayManual();
        break;
    case true:
        configDisplayAutomatic();
        break;
    }
}
