#include <string.h>
#include "pico/stdlib.h"
#include "modules/display/ssd1306.h"
#include "modules/buttons/buttons.h"

// Definições de pinos
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_LINK 0x3C

#define VALUE_INCREMENT_VOLUME 5
#define DEFAULT_VALUE_DB 60
#define MIN_VALUE_DB 30

// Definições dos exemplos de som
#define NUM_FAIXAS 4
#define NUM_EXEMPLOS 4

// Declarações de variáveis globais
extern int valor_decibeis;
extern bool menu_active;
extern ssd1306_t ssd;
extern const char *niveis_ruido[NUM_FAIXAS][NUM_EXEMPLOS];

// Protótipos de funções
void init_display(void);
void all_options_config(void);
void define_draw_in_display_of_sound(int nivel_sound);
void format_sound_message(const char *sound);
void update_display_based_on_sound_level(int nivel_sound);
void menu_config(int option);
void menu_manual_config(void);
bool debounce(uint gpio);
char *int_to_string(int num);
const char *random_example_sound(int nivel_sound);