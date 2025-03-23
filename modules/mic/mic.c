#include <stdio.h>
#include "hardware/adc.h"
#include "mic.h"
#include "modules/core/dma.h"

// Buffer para as amostras e configuração do DMA
static uint16_t adc_buffer[SAMPLES];
static dma_adc_config_t adc_dma_config;

// Inicializa o Microfone
void init_mic(void)
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    // Configura o FIFO
    adc_fifo_setup(
        true,  // Habilita FIFO
        true,  // Habilita armazenamento dos dados
        1,     // Nível para disparo
        false, // Sem trigger de erro
        false  // Sem byte swap
    );

    dma_adc_init(&adc_dma_config);
}

// Mandando amostras via dma
void sample_mic(void)
{
    adc_fifo_drain();
    adc_run(false);
    dma_adc_start_transfer(&adc_dma_config, adc_buffer, SAMPLES);
    adc_run(true);
    dma_adc_wait_for_finish(&adc_dma_config);
    adc_run(false);
}

// Calcula o RMS (média dos valores de voltagem)
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

// Converte valor de voltagem para dB
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
