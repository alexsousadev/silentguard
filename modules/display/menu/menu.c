#include "modules/display/menu/menu.h"
#include "modules/mic/mic.h"
#include "modules/joystick/joystick.h"
#include "modules/display/display.h"

#define CALIBRATION_DURATION_MS 5000     // Tempo de calibração (ms)
#define CALIBRATION_CORRECTION_FACTOR 15 // Fator de correção para ajustar o valor de dB

int all_sample_count = 0;
int all_sum_db = 0;

// Função de intervalo de espera
void create_interval(uint32_t interval, float sum_db, int sample_count)
{
    absolute_time_t next_wake_time = delayed_by_us(get_absolute_time(), interval * 1000);
    while (!time_reached(next_wake_time))
    {
        sum_db += mic_get_db();
        sample_count++;
    }

    all_sum_db = sum_db;
    all_sample_count = sample_count;
}

// Exibe a mensagem de calibração
void display_calibration_message(const char *message)
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, message, 30, 30);
    ssd1306_send_data(&ssd);
}

// Realiza a calibração automática dos dB
void handle_automatic_config()
{
    float sum_db = 0;
    int sample_count = 0;
    float ambient_average = 0.0f;

    // Exibe mensagens de calibração no display
    display_calibration_message("Calibrando...");
    create_interval(CALIBRATION_DURATION_MS, sum_db, sample_count);

    display_calibration_message("Quase la...");
    create_interval(CALIBRATION_DURATION_MS, sum_db, sample_count);

    // Calcula a média e aplica o fator de correção
    ambient_average = (all_sum_db / all_sample_count) + CALIBRATION_CORRECTION_FACTOR;
    valor_decibeis = ambient_average;

    // Exibe o resultado da calibração no display
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Calibrado", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);
    ssd1306_draw_string(&ssd, "Limite: ", 15, 30);

    char *new_db = int_to_string(ambient_average);
    ssd1306_draw_string(&ssd, new_db, 75, 30);
    ssd1306_draw_string(&ssd, " dB", 94, 30);
    free(new_db);

    ssd1306_draw_string(&ssd, "Ja pode sair", 20, 50);
    ssd1306_send_data(&ssd);
    np_clear();
}

// Exibe e gerencia o menu manual de configuração
void handle_manual_config()
{
    menu_active = true;
    ssd1306_clear_screen(&ssd);menu_manual_config
    ();

    while (menu_active)
    {
        // Botão B: aumenta dB
        if (debounce(BUTTON_B))
        {
            valor_decibeis += VALUE_INCREMENT_VOLUME;
            if (valor_decibeis >= 105)
            {
                valor_decibeis = MIN_VALUE_DB;
            }
            menu_manual_config();
        }

        // Botão Joystick: sai do menu
        if (debounce(BUTTON_JOYSTICK))
        {
            menu_active = false;
        }
    }

    ssd1306_clear_screen(&ssd); // Limpa tela ao sair do menu
    ssd1306_send_data(&ssd);
}

// Seleciona a opção de configuração (manual ou automático)
void select_option_config(bool is_automatic)
{
    if (is_automatic)
    {
        handle_automatic_config();
    }
    else
    {
        handle_manual_config();
    }
}
