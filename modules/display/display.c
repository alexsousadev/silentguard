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

int valorDecibeis = 60; // Limite de dB configuráve

ssd1306_t ssd;           // Objeto do display
bool menuActive = false; // Flag de menu ativo/inativo

// Exemplos de ruídos por nível(para display)
const char *niveis_ruido[NUM_FAIXAS][NUM_EXEMPLOS + 1] = {
    {"nível 1", "Conversa normal", "Ar- condicionado", "Chuva leve"},
    {"nível 2", "Batedeira", "Secador de cabelo", "Trem"},
    {"nível 3", "Britadeira", "Serra eletrica", "Moto sem escapamento"},
    {"Silencio"}};

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
const char *randomSound(int nivelSound)
{
    int index = rand() % NUM_EXEMPLOS;
    return niveis_ruido[nivelSound - 1][index + 1];
}

// Desenha mensagem no display (quebrando em linhas se necessário)
void drawSoundMessage(const char *sound)
{
    char temp[50];
    strncpy(temp, sound, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *palavras[3]; // Max. 3 palavras por linha
    int count = 0;

    char *token = strtok(temp, " ");
    while (token != NULL && count < 3)
    {
        palavras[count++] = token;
        token = strtok(NULL, " ");
    }

    int y_start = 30;
    for (int i = 0; i < count; i++)
    {
        ssd1306_draw_string(&ssd, palavras[i], 30, y_start + (i * 10));
    }
}

// Define o que exibir no display baseado no nível de som
void defineDraw(int nivelSound)
{
    if (nivelSound == 4)
    {
        ssd1306_clear_screen(&ssd);
        drawSoundMessage(niveis_ruido[3][0]); // Exibe "Silencio" para nível 4
    }
    else
    {
        const char *sound = randomSound(nivelSound);
        drawSoundMessage(sound);
    }
}

// Exibe nível de som no display (com mensagem associada)
void defineDrawInDisplayOfSound(int nivelSound)
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Nivel de som", 20, 10);
    ssd1306_draw_string(&ssd, "--------", 35, 20);
    defineDraw(nivelSound);

    ssd1306_send_data(&ssd);
    sleep_ms(5000);
}

//==============================================================================
//                       CONFIGURAÇÕES DO MENU
//==============================================================================

// Controle das configurações do menu
void all_options_config()
{
    int num = 0;
    menuActive = true;
    ssd1306_clear_screen(&ssd);
    menu_config();
    int options[2] = {0, 1};
    bool option = 0;

    while (menuActive)
    {
        if (debounce(BUTTON_B)) // Botão B: Seleciona uma opção
        {
            select_option_config(option);
        }
        else if (debounce(BUTTON_A)) // Botão A: Escolhe opção
        {
            option = !option;

            if (options[option] == 0)
            {
                menu_config();
            }
            else
            {
                menu_config2();
            }
        }

        else if (debounce(BUTTON_JOYSTICK)) // Botão Joystick: Sai do menu
        {
            menuActive = false;
            return;
        }
    }
    ssd1306_clear_screen(&ssd); // Limpa tela ao sair do menu
}

// Opções do menu de configuração
void menu_config()
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Config", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);
    ssd1306_draw_string(&ssd, "-", 10, 30);
    ssd1306_draw_string(&ssd, "Manual", 30, 30);
    ssd1306_draw_string(&ssd, "Auto", 30, 40);
    ssd1306_send_data(&ssd);
}

void menu_config2()
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Config", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);
    ssd1306_draw_string(&ssd, "-", 10, 40);
    ssd1306_draw_string(&ssd, "Manual", 30, 30);
    ssd1306_draw_string(&ssd, "Auto", 30, 40);
    ssd1306_send_data(&ssd);
}

// Desenha a tela de configuração do menu
void drawMenuScreen()
{
    ssd1306_clear_screen(&ssd);
    ssd1306_draw_string(&ssd, "Config", 36, 10);
    ssd1306_draw_string(&ssd, "--------", 30, 20);
    ssd1306_draw_string(&ssd, "Volume:", 10, 30);
    char *volumeStr = intToString(valorDecibeis);
    ssd1306_draw_string(&ssd, volumeStr, 70, 30);
    ssd1306_draw_string(&ssd, " dB", 95, 30);
    free(volumeStr);
    ssd1306_send_data(&ssd);
}

// Tratar debounce de um botão
bool debounce(uint gpio)
{
    static uint64_t last_press_time = 0;  // último clique
    uint64_t current_time = time_us_64(); // tempo atual

    if (gpio_get(gpio) == 0)
    {
        if ((current_time - last_press_time) > 200000) // Espera 200ms
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

// Converte inteiro para string (para display)
char *intToString(int num)
{
    char *str = (char *)malloc(12 * sizeof(char));
    if (str == NULL)
        return NULL;
    snprintf(str, 12, "%d", num);
    return str;
}
