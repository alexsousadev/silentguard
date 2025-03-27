#include "pico/stdlib.h"
#include "modules/matriz_leds/matriz_leds.h"
#include "modules/buzzer/buzzer.h"
#include "modules/display/display.h"
#include "modules/joystick/joystick.h"
#include "modules/interruptions/interruptions.h"
#include "modules/mic/mic.h"
#include "modules/buttons/buttons.h"
#include "modules/leds/leds.h"
#include "modules/core/dma.h"

//==============================================================================
// Definições e inclusões
//==============================================================================

// Variáveis auxiliares para a fórmula do EMA
typedef struct
{
    int noise_flag;
    int max_warning_flag;
    float noise_flag_smoothed;
    float smoothing_factor;
    int min_flag_threshold;
    int max_flag_threshold;
    uint64_t last_warning_time;
    uint64_t last_increment_time;
} NoiseSmoothed;

NoiseSmoothed noise_data = {
    .noise_flag = 0,
    .max_warning_flag = 0,
    .noise_flag_smoothed = 0.0f,
    .smoothing_factor = 2.0f,
    .min_flag_threshold = 1,
    .max_flag_threshold = 10,
    .last_warning_time = 0,
    .last_increment_time = 0};

const uint64_t decrement_delay_us = 20000000;  // Tempo para reduzir aviso (20 segundos)
const uint64_t increment_cooldown_us = 500000; // Cooldown de 0.5 segundos para incremento

//==============================================================================
// Protótipos de funções
//==============================================================================

void init_all_gpios();
void define_action(float db_value);
void warning_sound(int sound_level);
void warning_loop();
int define_noise_level(float avg_digital);
void define_noise_flag(float avg_digital);

//==============================================================================
// Funções de inicialização
//==============================================================================

// Inicializa todos os GPIOs
void init_all_gpios()
{
    init_leds();        // LEDs
    init_matriz_leds(); // Matriz de LEDs
    init_display();     // Display geral
    init_buzzer();      // Buzzer
    init_buttons();     // Botões
    init_joystick();    // Joystick
    init_mic();         // Microfone
}

//==============================================================================
// Tratamento do nível do som e flags
//==============================================================================

// Classifica nível de som (1: baixo, 2: alto, 3: muito alto)
int define_noise_level(float avg_digital)
{
    if (avg_digital >= 90.0) // Som muito alto
    {
        return 3;
    }
    else if (avg_digital >= menu_config.valor_decibeis) // Som alto (limiar configurável)
    {
        return 2;
    }
    else // Som baixo
    {
        return 1;
    }
}

// Define a flag de barulho com base no nível de som
void define_noise_flag(float avg_digital)
{
    int sound_level = define_noise_level(avg_digital);
    uint64_t current_time = time_us_64();
    float target = 0.0f;

    switch (sound_level)
    {
    case 1: // Som baixo
        target = 0.0;
        if (noise_data.max_warning_flag > 0 && (current_time - noise_data.last_warning_time >= decrement_delay_us))
        {
            noise_data.max_warning_flag--;
            noise_data.last_warning_time = current_time;
        }
        break;
    case 2: // Som médio
        target = 1.0;
        noise_data.last_warning_time = current_time;
        break;
    case 3: // Som alto
        target = 3.0;
        noise_data.last_warning_time = current_time;
        break;
    }

    // Atualiza com suavização exponencial (Exponential Moving Average)
    noise_data.noise_flag_smoothed = noise_data.noise_flag_smoothed * (1 - noise_data.smoothing_factor) + target * noise_data.smoothing_factor;

    if (noise_data.noise_flag_smoothed < noise_data.min_flag_threshold)
        noise_data.noise_flag_smoothed = noise_data.min_flag_threshold;
    if (noise_data.noise_flag_smoothed > noise_data.max_flag_threshold)
        noise_data.noise_flag_smoothed = noise_data.max_flag_threshold;

    noise_data.noise_flag = (int)noise_data.noise_flag_smoothed;
}

// Define ações baseadas no nível de som (dB)
void update_leds(void)
{
    if (noise_data.max_warning_flag == 2)
    {
        // Indica alerta: LED vermelho
        gpio_put(LEDs[0], 0);
        gpio_put(LEDs[1], 1);
    }
    else if (noise_data.noise_flag == 1)
    {
        // Estado intermediário: LED azul
        gpio_put(LEDs[0], 1);
        gpio_put(LEDs[1], 0);
    }
    else
    {
        // Sem alerta: LEDs apagados
        gpio_put(LEDs[0], 0);
        gpio_put(LEDs[1], 0);
    }
}

// Definição das ações baseadas no nível de som (dB)
void define_action(float db_value)
{
    int level = define_noise_level(db_value); // Classifica nível de som
    define_noise_flag(level);                 // Faz o controle das flags de barulho

    // Controla a matriz de LEDs com base no som
    update_leds();

    // Emite o som de aviso
    warning_sound(level);
}

//==============================================================================
// Aviso sonoro e visual
//==============================================================================

// Ações de aviso (display e som) por nível de som
void warning_sound(int sound_level)
{
    uint64_t current_time = time_us_64();
    if (sound_level >= 2 && (current_time - noise_data.last_increment_time >= increment_cooldown_us))
    {
        noise_data.max_warning_flag++;
        noise_data.last_increment_time = current_time; // Atualiza o tempo do último incremento
    }

    // Define o desenho da matriz de leds baseado no nível de som (Emoji feliz ou triste)
    if (sound_level < 2)
    {
        happy_emoji();
    }
    else
    {
        sad_emoji();
    }

    define_draw_in_display_of_sound(sound_level);
}

// Loop de aviso final (alarme máximo)
void warning_loop()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            sad_emoji();
            gpio_put(LEDs[1], 1);
            beep_buzzer(2000, 250);
            gpio_put(LEDs[1], 0);
            beep_buzzer(2000, 250);
        }
        sleep_ms(500);

        define_draw_in_display_of_sound(4); // Nível 4: "Silencio"
    }
}