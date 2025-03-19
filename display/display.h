#include "pico/stdlib.h"
#include "display/ssd1306.h"
#include "buttons/buttons.h"
#include <string.h>

// Definições de pinos
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_LINK 0x3C

#define VALUE_INCREMENT_VOLUME 5
#define DEFAULT_VALUE_DB 60

#define NUM_FAIXAS 4
#define NUM_EXEMPLOS 3

// Declarações de variáveis globais
extern int valorDecibeis;
extern bool menuActive;
extern ssd1306_t ssd;
extern const char *niveis_ruido[NUM_FAIXAS][NUM_EXEMPLOS + 1];

// Protótipos de funções
void init_display(void);
void drawSoundMessage(const char *sound);
void defineDraw(int nivelSound);
void defineDrawInDisplayOfSound(int nivelSound);
void drawMenuScreen(void);
void menu_config(void);
void configDisplayConfig(void);
void button_a_callback(uint gpio, uint32_t events);
bool debounce(uint gpio);
char *intToString(int num);
const char *randomSound(int nivelSound);