#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "matriz_leds/matriz_leds.h"
#include "buzzer/buzzer.h"
#include "display/display.h"
#include "joystick/joystick.h"
#include "interruptions/interruptions.h"
#include <math.h>

//==============================================================================
//                       DEFINIÇÕES E INCLUSÕES
//==============================================================================

// Pino e canal do microfone
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

// Amostras para cálculo do nível de ruído
#define SAMPLES 100

// LEDs indicadores: Azul e vermelho
const uint8_t LEDS[] = {12, 13};

// Constantes para conversão para dB
#define ADC_MAX 4095
#define VREF 3.3
#define OFFSET 2048
#define SENSITIVITY 0.00631 // VRMS/Pa do microfone
#define GAIN 20

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

// Buffer para amostras do ADC
uint16_t adc_buffer[SAMPLES];

// DMA configuration
uint dma_channel;
dma_channel_config dma_cfg;

//==============================================================================
//                       PROTÓTIPOS DE FUNÇÕES
//==============================================================================

void init_all_gpios();
void init_adc();
void init_leds();
void dma_config();

float calculate_rms(uint16_t *samples, int num_samples);
float adc_to_db(float vrms);
void sample_mic();
float read_mic();

int treatmentNivelOfSound(float avgDigital);
void defineFlagSound(float avgDigital);
void defineAction(float avg_digital_dB);
void warningSound(int nivelSound);
void loop_aviso();

//==============================================================================
//                       FUNÇÕES DE CONVERSÃO EM dB
//==============================================================================

// Calcula RMS de um buffer de amostras ADC
float calculate_rms(uint16_t *samples, int num_samples)
{
  float sum = 0.0;
  for (int i = 0; i < num_samples; i++)
  {
    float voltage = (samples[i] - OFFSET) * (VREF / ADC_MAX);
    sum += voltage * voltage;
  }
  return sqrt(sum / num_samples);
}

// Converte tensão RMS para Decibéis (dB)
float adc_to_db(float vrms)
{
  float vrms_with_gain = vrms * GAIN;
  float adjusted_sensitivity = SENSITIVITY * 0.5; // Ajuste fino da sensibilidade

  return 20 * log10(vrms_with_gain / adjusted_sensitivity);
}

//==============================================================================
//                       FUNÇÕES DE INICIALIZAÇÃO
//==============================================================================

// Configura e Inicializa o ADC para microfone
void init_adc()
{
  adc_gpio_init(MIC_PIN);
  adc_init();
  adc_select_input(MIC_CHANNEL);

  adc_fifo_setup(
      true,
      true,
      1,
      false,
      false);
}

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
}

// // Inicialização das Interrupções
// void init_all_interrupts()
// {
// }

// Configura DMA para transferência ADC
void dma_config()
{
  dma_channel = dma_claim_unused_channel(true);
  dma_cfg = dma_channel_get_default_config(dma_channel);

  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&dma_cfg, false);
  channel_config_set_write_increment(&dma_cfg, true);
  channel_config_set_dreq(&dma_cfg, DREQ_ADC);

  channel_config_set_ring(&dma_cfg, false, 0); // Sem "wrap"
}

//==============================================================================
//                       LEITURA DO MICROFONE
//==============================================================================

// Captura bloco de amostras do microfone via DMA
void sample_mic()
{
  adc_fifo_drain();
  adc_run(false);

  dma_channel_configure(
      dma_channel,
      &dma_cfg,
      adc_buffer,
      &adc_hw->fifo,
      SAMPLES,
      true);

  adc_run(true);
  dma_channel_wait_for_finish_blocking(dma_channel);
  adc_run(false);
}

// Calcula média das leituras do microfone (no buffer)
float read_mic()
{
  float avg = 0.f;
  for (uint i = 0; i < SAMPLES; ++i)
  {
    avg += (uint)adc_buffer[i];
  }
  avg /= SAMPLES;
  return avg;
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
void defineAction(float avg_digital_dB)
{
  int nivel = treatmentNivelOfSound(avg_digital_dB);

  defineFlagSound(avg_digital_dB);

  if (flag_aviso_maximo == 2)
  {
    gpio_put(LEDS[0], 0);
    gpio_put(LEDS[1], 1);
    // beepBuzzer(2000, 250);
    // gpio_put(LEDS[1], 0);
    // beepBuzzer(2000, 250);
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
  init_adc();
  init_all_gpios();
  init_interruptions();
  // set_interrupts();
  // init_all_interrupts();
  dma_config();

  while (true)
  {
    sample_mic();
    float avg = read_mic(); // Média das amostras (para debug, valor dB é usado)

    float vrms = calculate_rms(adc_buffer, SAMPLES);
    float db_value = adc_to_db(vrms);
    defineAction(db_value); // Ações baseadas no nível de dB
    sleep_ms(100);
  }
}