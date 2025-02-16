#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "neopixel.h"
#include "buzzer.h"
#include "display.h"
#include "delay.h"

// Definição do canal do microfone e pino ADC
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

// Número de amostras para o cálculo
#define SAMPLES 100

// Flags de aviso
int flag_barulho = 0;
int flag_aviso_maximo = 0;

// Definição dos LEDs
const uint8_t LEDS[] = {11, 12, 13}; // Leds verde, azul e vermelho

// Buffer para armazenar as amostras do ADC
uint16_t adc_buffer[SAMPLES];

// Canal do DMA
uint dma_channel;
dma_channel_config dma_cfg;

void init_all_gpios();
float read_mic();
void init_adc();
void dma_config();
void sample_mic();
void defineAction(float avg_digital);

int main()
{
  // Inicializa as funções de entrada/saída padrão
  stdio_init_all();
  // init_buzzer();
  init_adc();
  init_all_gpios();
  dma_config();

  while (true)
  {
    sample_mic();                        // Captura os dados usando DMA
    float avg = read_mic();              // Calcula a média das amostras
    float voltagem = avg * (3.3 / 4096); // Converte para tensão
    sleep_ms(10);
    printf("\nValor analógico: %.2f V\n", voltagem);
    printf("Valor digital: %.2f\n", avg);
    printf("Valor do barulho: %d\n", flag_barulho);

    defineAction(avg); // Define a ação a ser tomada com base no som
  }
}

// Configuração do ADC e Inicialização do ADC
void init_adc()
{
  adc_gpio_init(MIC_PIN);
  adc_init();
  adc_select_input(MIC_CHANNEL);

  // Configurar o ADC para modo FIFO
  adc_fifo_setup(
      true,  // Habilitar FIFO
      true,  // Habilitar DMA
      1,     // Threshold FIFO (pode ajustar conforme necessário)
      false, // Não descartar leituras antigas
      true   // Modo ERG (conversão contínua)
  );
}

// Inicialização dos LEDs
void init_leds()
{
  for (int i = 0; i < 3; i++)
  {
    gpio_init(LEDS[i]);
    gpio_set_dir(LEDS[i], GPIO_OUT);
  }
}

void loop_aviso();

// Tratamentos diferentes dependendo do nível do som (comum ou intenso)
// A aplicação possui 3 níveis
int treatmentNivelOfSound(float avgDigital)
{
  if (avgDigital >= 200.0)
  {
    return 3; // Som muito alto
  }
  else if (avgDigital >= 133.0)
  {
    return 2; // Som médio
  }
  else
  {
    return 1; // Som baixo
  }
}

float flag_barulho_smoothed = 0.0; // Variável para armazenar o valor suavizado da flag de barulho
float smoothing_factor = 2;        // Aumentando o fator de suavização (aumente conforme necessário)
int min_flag_threshold = 1;        // Limite mínimo para o valor da flag (para evitar que caia para 0)
int max_flag_threshold = 10;       // Limite máximo para a flag (pode ser ajustado conforme necessário)

void defineFlagSound(float avgDigital)
{
  int sound_level = treatmentNivelOfSound(avgDigital);

  // Suavizar o aumento ou diminuição do barulho
  if (sound_level == 1) // Som baixo
  {
    // Diminui a flag suavemente
    flag_barulho_smoothed = flag_barulho_smoothed * (1 - smoothing_factor) + 0.0 * smoothing_factor;
  }
  else if (sound_level == 2) // Som médio
  {
    // Aumenta a flag de forma controlada
    flag_barulho_smoothed = flag_barulho_smoothed * (1 - smoothing_factor) + 1.0 * smoothing_factor;
  }
  else if (sound_level == 3) // Som alto
  {
    // Aumenta a flag mais rápido
    flag_barulho_smoothed = flag_barulho_smoothed * (1 - smoothing_factor) + 3.0 * smoothing_factor;
  }

  // Controla a flag para não ultrapassar os limites
  if (flag_barulho_smoothed < min_flag_threshold)
    flag_barulho_smoothed = min_flag_threshold;
  if (flag_barulho_smoothed > max_flag_threshold)
    flag_barulho_smoothed = max_flag_threshold;

  // Armazena a flag arredondada
  flag_barulho = (int)flag_barulho_smoothed;

  // Debug: Imprime o valor da flag suavizada
  printf("Flag de barulho suavizada: %.2f\n", flag_barulho_smoothed);
  printf("Flag de barulho: %d\n", flag_barulho);
}

void warningSound(int nivelSound);

// Define o desenho do emoji
void defineAction(float avg_digital)
{
  // Define o nível de som (1, 2, 3)
  int nivel = treatmentNivelOfSound(avg_digital);

  // Atualiza a flag que sinaliza se o som está alto
  defineFlagSound(avg_digital);

  if (flag_aviso_maximo == 3)
  {
    loop_aviso();          // Chama a função de alarme
    flag_aviso_maximo = 0; // Reset a contagem de avisos consecutivos
  }

  // Define o que será feito com base no nível do som
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

// Ações com base no nível do som
void warningSound(int nivelSound)
{
  switch (nivelSound)
  {
  case 1:
    emojiFeliz();
    defineDrawInDisplayOfSound(nivelSound, flag_barulho);
    break;
  case 2:
    emojiTriste();
    defineDrawInDisplayOfSound(nivelSound, flag_barulho);
    flag_barulho = 0;
    flag_aviso_maximo++;
    break;
  case 3:
    emojiTriste();
    defineDrawInDisplayOfSound(nivelSound, flag_barulho);
    break;
  default:
    break;
  };
}
// Ações para o Aviso final
// Toca o alarme 3 vezes e mantém o emoji triste
void loop_aviso()
{
  // Toca o alarme 3 vezes
  for (int i = 0; i < 3; i++)
  {
    // Pisca o emoji 4 vezes por ciclo de alarme
    for (int j = 0; j < 4; j++)
    {
      emojiTriste();
      beepBuzzer(2000, 250); // Reduzido para 250ms
      npClear();             // Limpa o display para criar efeito de piscar
      beepBuzzer(2000, 250); // Continua o som
    }
    sleep_ms(500); // Pequena pausa entre cada ciclo
  }

  defineDrawInDisplayOfSound(4, flag_barulho);
}

// Inicialização de todos os pinos
void init_all_gpios()
{
  init_leds();        // LEDs
  init_matriz_leds(); // Matriz de LEDs
  init_display();     // Display
  init_buzzer();      // Buzzer
}

// Configuração do DMA para captura automática de amostras
void dma_config()
{
  dma_channel = dma_claim_unused_channel(true);
  dma_cfg = dma_channel_get_default_config(dma_channel);

  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&dma_cfg, false);
  channel_config_set_write_increment(&dma_cfg, true);
  channel_config_set_dreq(&dma_cfg, DREQ_ADC);

  // Configurar para não fazer wrap (parar após preencher o buffer)
  channel_config_set_ring(&dma_cfg, false, 0);
}

// Captura amostras usando DMA
void sample_mic()
{
  adc_fifo_drain(); // Limpa o FIFO
  adc_run(false);   // Para o ADC

  // Reseta o ponteiro de escrita do DMA
  dma_channel_configure(
      dma_channel,
      &dma_cfg,
      adc_buffer,    // destino
      &adc_hw->fifo, // origem
      SAMPLES,       // quantidade
      true           // start immediately
  );

  adc_run(true); // Inicia o ADC

  // Espera a transferência completar
  dma_channel_wait_for_finish_blocking(dma_channel);

  adc_run(false); // Para o ADC
}

// Calcula a média simples das leituras do microfone
float read_mic()
{
  float avg = 0.f;
  for (uint i = 0; i < SAMPLES; ++i)
  {
    avg += (uint)adc_buffer[i]; // Converte para unsigned int antes de somar
  }
  avg /= SAMPLES; // Divide pelo número de amostras para obter a média
  return avg;
}