// Bibliotecas base
#include <stdio.h>
#include "pico/stdlib.h"
#include "display/ssd1306.h" //funções de manipulação do display
#include "display/font.h"    //Fonte das letras apresentadas no display
#include "delay.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_LINK 0x3C

ssd1306_t ssd;

// Definição dos níveis de barulho
// 1 - Conversa Comum, 2 - Transito Intenso, 3 - Motosserra
const char *niveisOfSound[3] = {"Conversa Comum", "Transito", "Motosserra"};

void defineDraw(int option)
{

    switch (option)
    {
    case 1:
        ssd1306_draw_string(&ssd, "Conversa ", 30, 20);
        ssd1306_draw_string(&ssd, " Comum ", 40, 30);
        break;
    case 2:
        ssd1306_draw_string(&ssd, niveisOfSound[1], 30, 20);
        break;
    case 3:
        ssd1306_draw_string(&ssd, niveisOfSound[2], 30, 20);
    case 4:
        ssd1306_draw_string(&ssd, "Faca", 30, 20);
        ssd1306_draw_string(&ssd, "Silencio", 40, 30);
    default:
        break;
    }
}

// Define um texto na tela com base no nível do Som
void defineDrawInDisplayOfSound(int nivelSound, int num_avisos)
{
    ssd1306_clear_screen(&ssd);                        // Limpa a tela
    ssd1306_draw_string(&ssd, "Nivel de som", 10, 10); // Texto padrão para todos
    switch (nivelSound)
    {
    case 1:
        defineDraw(1); // "Conversa Comum"
        break;
    case 2:
        defineDraw(2); // "Transito"
        break;
    case 3:
        defineDraw(3); // "Motosserra"
        break;
    case 4:
        ssd1306_clear_screen(&ssd);
        defineDraw(4); // "Faca Silencio"
        break;
    default:
        break;
    }
    ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);
    ssd1306_send_data(&ssd);
    sleep_ms(5000);
}

// Inicialização do display
void init_display()
{
    // Inicializa o i2c, inicializa a estrutura do display, limpa o display
    ssd1306_init_config_clean(&ssd, I2C_SCL, I2C_SDA, I2C_PORT, I2C_LINK);
}
