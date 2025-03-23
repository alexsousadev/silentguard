#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "modules/display/ssd1306.h"
#include "modules/display/display.h"
#include "modules/display/menu/menu.h"
#include "modules/joystick/joystick.h"

//==============================================================================
//                       VARIÁVEIS GLOBAIS
//==============================================================================

int valor_decibeis = 60; // Limite de dB configurável

ssd1306_t ssd;            // Objeto do display
bool menu_active = false; // Flag de menu ativo/inativo

const char *niveis_ruido[NUM_FAIXAS][NUM_EXEMPLOS] = {
    // Nível 1 (≤60 dB) – Sons baixos
    {"Maquina de Lavar", "Relogio", "Conversa normal", "Chuva leve"},

    // Nível 2 (61 - 90 dB) – Sons moderados
    {"Batedeira", "Aspirador", "Trem", "Cortador de grama"},

    // Nível 3 (>90 dB) – Sons altos
    {"Show de Rock", "Serra eletrica", "Sirene", "Furadeira"}};

//==============================================================================
//                       FUNÇÕES DE INICIALIZAÇÃO
//==============================================================================

void init_display()
{
    ssd1306_init_config_clean(&ssd, I2C_SCL, I2C_SDA, I2C_PORT, I2C_LINK);
}

//==============================================================================
//                       FUNÇÕES RELACIONADAS AO DISPLAY
//==============================================================================

// Escolhe um som aleatório para exibir (por nível)
const char *random_example_sound(int nivel_sound)
{
    int index = rand() % NUM_EXEMPLOS;
    return niveis_ruido[nivel_sound - 1][index];
}

// Desenha a mensagem no display (quebrando em linhas se necessário)
void format_sound_message(const char *sound)
{
    char buffer[50];
    strncpy(buffer, sound, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    int y_position = 30;
    char *word = strtok(buffer, " ");
    for (int i = 0; word != NULL && i < 3; i++)
    {
        ssd1306_draw_string(&ssd, word, 30, y_position + (i * 10));
        word = strtok(NULL, " ");
    }
}

// Define o que exibir no display baseado no nível de som
void update_display_based_on_sound_level(int nivel_sound)
{
    if (nivel_sound == 4)
    {
        ssd1306_clear_screen(&ssd);
        ssd1306_draw_string(&ssd, "Silencio", 30, 30);
    }
    else
    {
        const char *sound = random_example_sound(nivel_sound);
        format_sound_message(sound);
    }
}

// Exibe nível de som no display
void define_draw_in_display_of_sound(int nivel_sound)
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Nivel de som", 20, 10);
    ssd1306_draw_string(&ssd, "--------", 35, 20);
    update_display_based_on_sound_level(nivel_sound);
    ssd1306_send_data(&ssd);
    sleep_ms(5000);
}

//==============================================================================
//                       CONFIGURAÇÕES DO MENU
//==============================================================================

// Opções do menu de configuração
void define_option_of_menu_config(int option)
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Config", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);

    ssd1306_draw_string(&ssd, (option == 0) ? "-" : " ", 10, 30);
    ssd1306_draw_string(&ssd, "Manual", 30, 30);

    ssd1306_draw_string(&ssd, (option == 1) ? "-" : " ", 10, 40);
    ssd1306_draw_string(&ssd, "Auto", 30, 40);

    ssd1306_send_data(&ssd);
}

// Controle das configurações do menu
void all_options_config()
{
    int option = 0;
    menu_active = true;
    ssd1306_clear_screen(&ssd);
    define_option_of_menu_config(option);

    while (menu_active)
    {
        if (debounce(BUTTON_B))
        {
            select_option_config(option);
        }
        else if (debounce(BUTTON_A))
        {
            option = !option;
            define_option_of_menu_config(option);
        }
        else if (debounce(BUTTON_JOYSTICK))
        {
            menu_active = false;
            return;
        }
    }

    ssd1306_clear_screen(&ssd);
}

// Desenha a tela de configuração do menu
void menu_manual_config()
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Config", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);
    ssd1306_draw_string(&ssd, "Volume:", 10, 30);
    char *volume_str = int_to_string(valor_decibeis);
    ssd1306_draw_string(&ssd, volume_str, 70, 30);
    ssd1306_draw_string(&ssd, " dB", 95, 30);
    free(volume_str);
    ssd1306_send_data(&ssd);
}

// Tratar debounce de um botão
bool debounce(uint gpio)
{
    static uint64_t last_press_time = 0;
    uint64_t current_time = time_us_64();

    if (gpio_get(gpio) == 0)
    {
        if ((current_time - last_press_time) > 200000)
        {
            last_press_time = current_time;
            return true;
        }
    }
    return false;
}

//==============================================================================
//                       FUNÇÕES UTILITÁRIAS
//==============================================================================

// Converte inteiro para string
char *int_to_string(int num)
{
    char *str = (char *)malloc(12 * sizeof(char));
    if (str == NULL)
        return NULL;
    snprintf(str, 12, "%d", num);
    return str;
}