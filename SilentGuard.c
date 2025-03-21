#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "modules/matriz_leds/matriz_leds.h"
#include "modules/buzzer/buzzer.h"
#include "modules/display/display.h"
#include "modules/joystick/joystick.h"
#include "modules/interruptions/interruptions.h"
#include "modules/core/dma.h"
#include "modules/mic/mic.h"

//==============================================================================
//                       DEFINIÇÕES E INCLUSÕES
//==============================================================================

// LEDs indicadores: Azul e vermelho
const uint8_t LEDS[] = {12, 13};

// Flags e tempos de aviso
int flag_barulho = 0;
int flag_aviso_maximo = 0;
const uint64_t DECREMENT_DELAY_US = 20000000; // Tempo para reduzir aviso (20 segundos)

// Variáveis de suavização da flag de barulho
float flag_barulho_smoothed = 0.0;
float smoothing_factor = 2;
int min_flag_threshold = 1;
int max_flag_threshold = 10;
uint64_t last_warning_time = 0;
uint64_t last_increment_time = 0;              // Tempo do último incremento de aviso
const uint64_t INCREMENT_COOLDOWN_US = 500000; // Cooldown de 0.5 segundos para incremento

//==============================================================================
//                       PROTÓTIPOS DE FUNÇÕES
//==============================================================================

void init_all_gpios();

void init_leds();

void defineAction(float db_value);
void warningSound(int nivelSound);
void loop_aviso();

//==============================================================================
//                       FUNÇÕES DE INICIALIZAÇÃO
//==============================================================================

// Inicializa LEDs indicadores
void init_leds()
{
  for (int i = 0; i < 2; i++)
  {
    gpio_init(LEDS[i]);
    gpio_set_dir(LEDS[i], GPIO_OUT);
    gpio_put(LEDS[i], 0);
  }
}

// Inicializa todos os GPIOs: LEDs, display, buzzer
void init_all_gpios()
{
  init_leds();
  init_matriz_leds(); // Display de matriz
  init_display();     // Display geral
  init_buzzer();      // Buzzer
  buttons_init();     // Botões
  init_joystick();    // Joystick
  mic_init();
}

//==============================================================================
//                       TRATAMENTO DO NÍVEL DE SOM E FLAGS
//==============================================================================

// Classifica nível de som (1: baixo, 2: médio, 3: alto)
int treatmentNivelOfSound(float avgDigital)
{
  if (avgDigital >= 80.0) // Som muito alto
  {
    return 3;
  }
  else if (avgDigital >= valorDecibeis) // Som alto (limiar configurável)
  {
    return 2;
  }
  else // Som baixo
  {
    return 1;
  }
}

// Define flag de barulho com suavização
void defineFlagSound(float avgDigital)
{
  int sound_level = treatmentNivelOfSound(avgDigital);

  if (sound_level == 1) // Som baixo
  {
    flag_barulho_smoothed = flag_barulho_smoothed * (1 - smoothing_factor) + 0.0 * smoothing_factor;
    if (flag_aviso_maximo > 0)
    {
      uint64_t current_time = time_us_64();
      if (current_time - last_warning_time >= DECREMENT_DELAY_US)
      {
        flag_aviso_maximo--;
        last_warning_time = current_time;
      }
    }
  }
  else if (sound_level == 2) // Som médio
  {
    flag_barulho_smoothed = flag_barulho_smoothed * (1 - smoothing_factor) + 1.0 * smoothing_factor;
    last_warning_time = time_us_64();
  }
  else if (sound_level == 3) // Som alto
  {
    flag_barulho_smoothed = flag_barulho_smoothed * (1 - smoothing_factor) + 3.0 * smoothing_factor;
    last_warning_time = time_us_64();
  }

  if (flag_barulho_smoothed < min_flag_threshold)
    flag_barulho_smoothed = min_flag_threshold;
  if (flag_barulho_smoothed > max_flag_threshold)
    flag_barulho_smoothed = max_flag_threshold;

  flag_barulho = (int)flag_barulho_smoothed;
}

// Define ações baseadas no nível de som (dB)
void defineAction(float db_value)
{
  int nivel = treatmentNivelOfSound(db_value);
  defineFlagSound(db_value);

  if (flag_aviso_maximo == 2)
  {
    gpio_put(LEDS[0], 0);
    gpio_put(LEDS[1], 1);
  }
  else if (flag_barulho == 1 && flag_aviso_maximo < 2)
  {
    gpio_put(LEDS[0], 1); // Azul: Aviso de barulho
    gpio_put(LEDS[1], 0);
  }
  else
  {
    gpio_put(LEDS[0], 0);
    gpio_put(LEDS[1], 0);
  }

  if (flag_aviso_maximo == 3)
  {
    loop_aviso();
    flag_aviso_maximo = 0;
  }

  switch (nivel)
  {
  case 1:
    warningSound(nivel);
    break;
  case 2:
    warningSound(nivel);
    break;
  case 3:
    warningSound(nivel);
    break;
  default:
    break;
  }
}

//==============================================================================
//                       AVISO SONORO E VISUAL
//==============================================================================

// Ações de aviso (display e som) por nível de som
void warningSound(int nivelSound)
{
  uint64_t current_time = time_us_64();
  if (nivelSound >= 2 && (current_time - last_increment_time >= INCREMENT_COOLDOWN_US))
  {
    flag_aviso_maximo++;
    last_increment_time = current_time; // Atualiza o tempo do último incremento
  }

  switch (nivelSound)
  {
  case 1: // Som baixo: Emoji feliz
    emojiFeliz();
    defineDrawInDisplayOfSound(nivelSound);
    break;
  case 2: // Som médio: Emoji triste, incrementa aviso
    emojiTriste();
    defineDrawInDisplayOfSound(nivelSound);

    break;
  case 3: // Som alto: Emoji triste, incrementa aviso
    emojiTriste();
    defineDrawInDisplayOfSound(nivelSound);
    break;
  default:
    break;
  };
}

// Loop de aviso final (alarme máximo)
void loop_aviso()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      emojiTriste();
      gpio_put(LEDS[1], 1);
      beepBuzzer(2000, 250);
      gpio_put(LEDS[1], 0);
      beepBuzzer(2000, 250);
    }
    sleep_ms(500);

    defineDrawInDisplayOfSound(4); // Nível 4: "Silencio" (alarme final)
  }
}

//==============================================================================
//                       FUNÇÃO PRINCIPAL
//==============================================================================
int main()
{
  stdio_init_all();
  init_all_gpios();
  init_interruptions();

  while (true)
  {
    float db_value = mic_get_db();
    printf("Valor dB: %f\n", db_value);
    defineAction(db_value); // Ações baseadas no nível de dB
    sleep_ms(100);
  }
}
