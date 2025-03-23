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

// Flags e tempos de aviso
int noise_flag = 0;
int max_warning_flag = 0;
const uint64_t decrement_delay_us = 20000000; // Tempo para reduzir aviso (20 segundos)

// Variáveis de suavização da flag de barulho
float noise_flag_smoothed = 0.0;
float smoothing_factor = 2;
int min_flag_threshold = 1;
int max_flag_threshold = 10;
uint64_t last_warning_time = 0;
uint64_t last_increment_time = 0;              // Tempo do último incremento de aviso
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

// Inicializa todos os GPIOs: LEDs, display, buzzer
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

// Classifica nível de som (1: baixo, 2: médio, 3: alto)
int define_noise_level(float avg_digital)
{
  if (avg_digital >= 90.0) // Som muito alto
  {
    return 3;
  }
  else if (avg_digital >= valor_decibeis) // Som alto (limiar configurável)
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
    if (max_warning_flag > 0 && (current_time - last_warning_time >= decrement_delay_us))
    {
      max_warning_flag--;
      last_warning_time = current_time;
    }
    break;
  case 2: // Som médio
    target = 1.0;
    last_warning_time = current_time;
    break;
  case 3: // Som alto
    target = 3.0;
    last_warning_time = current_time;
    break;
  }

  // Atualiza com suavização exponencial (Exponential Moving Average)
  noise_flag_smoothed = noise_flag_smoothed * (1 - smoothing_factor) + target * smoothing_factor;

  if (noise_flag_smoothed < min_flag_threshold)
    noise_flag_smoothed = min_flag_threshold;
  if (noise_flag_smoothed > max_flag_threshold)
    noise_flag_smoothed = max_flag_threshold;

  noise_flag = (int)noise_flag_smoothed;
}

// Define ações baseadas no nível de som (dB)
void define_action(float db_value)
{
  int level = define_noise_level(db_value); // Classifica nível de som

  define_noise_flag(level); // Faz o controle das flags de barulho

  if (max_warning_flag == 2)
  {
    gpio_put(LEDs[0], 0);
    gpio_put(LEDs[1], 1); // Vermelho
  }
  else if (noise_flag == 1 && max_warning_flag < 2)
  {
    gpio_put(LEDs[0], 1); // Azul
    gpio_put(LEDs[1], 0);
  }
  else
  {
    gpio_put(LEDs[0], 0); // Apagado
    gpio_put(LEDs[1], 0);
  }

  if (max_warning_flag == 3)
  {
    warning_loop();
    max_warning_flag = 0;
  }
  warning_sound(level);
}

//==============================================================================
// Aviso sonoro e visual
//==============================================================================

// Ações de aviso (display e som) por nível de som
void warning_sound(int sound_level)
{
  uint64_t current_time = time_us_64();
  if (sound_level >= 2 && (current_time - last_increment_time >= increment_cooldown_us))
  {
    max_warning_flag++;
    last_increment_time = current_time; // Atualiza o tempo do último incremento
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

//==============================================================================
// Função Principal
//==============================================================================
int main()
{
  stdio_init_all();
  init_all_gpios();
  init_interruptions();

  while (true)
  {
    float db_value = mic_get_db();
    define_action(db_value); // Ações baseadas no nível de dB
    sleep_ms(100);
  }
}
