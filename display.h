#include <stdio.h>
#include "pico/stdlib.h"
#include "display/ssd1306.h"
#include <stdlib.h>
#include <string.h>

//==============================================================================
//                       DEFINIÇÕES E INCLUSÕES
//==============================================================================

// Definições I2C para display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_LINK 0x3C // Endereço do display

// Pinos dos botões A e B
#define BUTTON_A 5
#define BUTTON_B 6

// Ajuste de volume e valor padrão de dB
#define VALUE_INCREMENT_VOLUME 5
#define DEFAULT_VALUE_DB 60

// Níveis de ruído de exemplo (para display)
#define NUM_FAIXAS 4
#define NUM_EXEMPLOS 3

//==============================================================================
//                       VARIÁVEIS GLOBAIS
//==============================================================================

int buttons[2] = {BUTTON_A, BUTTON_B}; // Pinos dos botões
int valorDecibeis = 60;                // Limite de dB configurável (padrão)

ssd1306_t ssd;           // Driver do display OLED
bool menuActive = false; // Flag de menu ativo/inativo

// Exemplos de ruídos por nível (para display)
const char *niveis_ruido[NUM_FAIXAS][NUM_EXEMPLOS + 1] = {
    {"nível 1", "Conversa normal", "Ar- condicionado", "Chuva leve"},
    {"nível 2", "Batedeira", "Secador de cabelo", "Trem"},
    {"nível 3", "Britadeira", "Serra eletrica", "Moto sem escapamento"},
    {"Silencio"}};

//==============================================================================
//                       PROTÓTIPOS DE FUNÇÕES
//==============================================================================

void init_display();
void buttons_init();
void set_interrupts();
void initialize_all();

void drawSoundMessage(const char *sound);
void defineDraw(int nivelSound);
void defineDrawInDisplayOfSound(int nivelSound);
void drawMenuScreen();

void configDisplayConfig();
void button_a_callback(uint gpio, uint32_t events);

bool debounce(uint gpio);
char *intToString(int num);

//==============================================================================
//                       FUNÇÕES DE INICIALIZAÇÃO
//==============================================================================

void init_display()
{
    ssd1306_init_config_clean(&ssd, I2C_SCL, I2C_SDA, I2C_PORT, I2C_LINK);
}

void buttons_init()
{
    for (int i = 0; i < 2; i++)
    {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }
}

// Configura interrupção para botão A (menu)
void set_interrupts()
{
    gpio_set_function(BUTTON_A, GPIO_FUNC_SIO);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_a_callback);
}

// Inicializa display, botões e interrupções
void initialize_all()
{
    init_display();
    buttons_init();
    set_interrupts();
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

    char *palavras[3]; // Max 3 palavras por linha
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
        drawSoundMessage(sound); // Exibe exemplo de som aleatório para o nível
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
    sleep_ms(5000); // Mensagem visível por 5 segundos
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

//==============================================================================
//                       MENU DE CONFIGURAÇÃO
//==============================================================================

// Callback do botão A: ativa/desativa menu de config
void button_a_callback(uint gpio, uint32_t events)
{
    if (!menuActive)
    {
        configDisplayConfig(); // Inicia menu se não ativo
    }
    else
    {
        menuActive = false; // Sai do menu se já ativo
    }
}

// Tela de configuração do display (volume em dB)
void configDisplayConfig()
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
                valorDecibeis = DEFAULT_VALUE_DB; // Reseta se passar de 105dB
            }
            drawMenuScreen();
        }

        if (debounce(BUTTON_A)) // Botão A: sai do menu
        {
            menuActive = false;
        }
    }
    ssd1306_clear_screen(&ssd); // Limpa tela ao sair do menu
    ssd1306_send_data(&ssd);
}

//==============================================================================
//                        TRATAMENTO DE BOTÕES
//==============================================================================

// Debounce para botões (evita múltiplos cliques)
bool debounce(uint gpio)
{
    static uint32_t last_press_time[2] = {0};
    int buttonIndex = (gpio == BUTTON_A) ? 0 : ((gpio == BUTTON_B) ? 1 : -1);
    if (buttonIndex == -1)
        return false;

    uint32_t current_time = get_absolute_time();
    if (gpio_get(gpio) == 0)
    {
        if (absolute_time_diff_us(last_press_time[buttonIndex], current_time) > 200000) // 200ms debounce
        {
            last_press_time[buttonIndex] = current_time;
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