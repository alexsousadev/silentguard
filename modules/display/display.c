#include "pico/stdlib.h"
#include "modules/display/ssd1306.h"
#include "modules/display/display.h"

//==============================================================================
// Variáveis Globais
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
// Exibição no Display
//==============================================================================

// Inicializa o display
void init_display()
{
    ssd1306_init_config_clean(&ssd, I2C_SCL, I2C_SDA, I2C_PORT, I2C_LINK);
}

// Escolhe um som aleatório para exibir (de acordo com o nível de Som)
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

// Converte inteiro para string
char *int_to_string(int num)
{
    char *str = (char *)malloc(12 * sizeof(char));
    if (str == NULL)
        return NULL;
    snprintf(str, 12, "%d", num);
    return str;
}