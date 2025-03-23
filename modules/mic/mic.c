#include <stdio.h>
#include <math.h>
#include "mic.h"
#include "hardware/adc.h"
#include "modules/core/dma.h"

// Definição do buffer de amostras e da configuração do DMA
static uint16_t adc_buffer[SAMPLES];
static dma_adc_config_t adc_dma_config;

// Inicializa o microfone (ADC)
void init_mic(void)
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    // Configuração do FIFO
    adc_fifo_setup(
        true, // Habilita o FIFO
        true, // Habilita armazenamento dos dados
        1,
        false, // Sem trigger de erro
        false  // Sem byte swap
    );

    // Inicializa a configuração do DMA
    dma_adc_init(&adc_dma_config);
}

// Envia amostras para o DMA
void sample_mic(void)
{
    adc_fifo_drain();                                             // Drena o FIFO
    adc_run(false);                                               // Desliga o ADC antes de iniciar a transferência
    dma_adc_start_transfer(&adc_dma_config, adc_buffer, SAMPLES); // Inicia a transferência do DMA
    adc_run(true);                                                // Liga o ADC para continuar a captura
    dma_adc_wait_for_finish(&adc_dma_config);                     // Espera a transferência terminar
    adc_run(false);                                               // Desliga o ADC após a transferência
}

// Calcula o valor RMS (Root Mean Square) a partir das amostras
float calculate_rms(const uint16_t *adc_buffer, uint32_t num_samples, float offset, float vref, float adc_max)
{
    float sum = 0.0;
    for (uint32_t i = 0; i < num_samples; i++)
    {
        float voltage = (adc_buffer[i] - offset) * (vref / adc_max);
        sum += voltage * voltage;
    }
    return sqrt(sum / num_samples);
}

// Converte o valor RMS (V) para decibéis (dB)
float adc_to_db(float vrms, float gain, float sensitivity)
{
    float vrms_with_gain = vrms * gain;
    float adjusted_sensitivity = sensitivity * 0.5;
    return 20 * log10(vrms_with_gain / adjusted_sensitivity);
}

// Função auxiliar que faz as leituras e traz o dB
float mic_get_db(void)
{
    sample_mic();
    float rms = calculate_rms(adc_buffer, SAMPLES, OFFSET, VREF, ADC_MAX);
    return adc_to_db(rms, GAIN, SENSITIVITY);
}
