#include <stdio.h>
#include "hardware/adc.h"
#include "mic.h"
#include "modules/core/dma.h" // Nosso módulo DMA para o ADC

// Buffer para as amostras e configuração do DMA
static uint16_t adc_buffer[SAMPLES];
static dma_adc_config_t adc_dma_config;

// Inicializa o Microfone
void mic_init(void)
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

// Ccalcular o RMS das amostras (média)
float calculate_rms(void)
{
    float sum = 0.0;
    for (int i = 0; i < SAMPLES; i++)
    {
        float voltage = (adc_buffer[i] - OFFSET) * (VREF / ADC_MAX);
        sum += voltage * voltage;
    }
    return sqrt(sum / SAMPLES);
}

// Converte o valor de voltagem para dB
float adc_to_db(float vrms)
{
    float vrms_with_gain = vrms * GAIN;
    float adjusted_sensitivity = SENSITIVITY * 0.5; // Ajuste fino da sensibilidade
    return 20 * log10(vrms_with_gain / adjusted_sensitivity);
}

// Pegando amostras pelo microfone
void sample_mic(void)
{
    adc_fifo_drain();
    adc_run(false);
    dma_adc_start_transfer(&adc_dma_config, adc_buffer, SAMPLES);
    adc_run(true);
    dma_adc_wait_for_finish(&adc_dma_config);
    adc_run(false);
}

float read_mic(void)
{
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES; i++)
    {
        printf("Valor: %d\n", adc_buffer[i]);
        avg += adc_buffer[i];
    }
    avg /= SAMPLES;
    return avg;
}

// Função auxiliar que faz as leituras e traz o dB
float mic_get_db(void)
{
    sample_mic();
    float rms = calculate_rms(); // Calculate RMS of the internal buffer
    return adc_to_db(rms);
}
