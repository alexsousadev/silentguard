#include <stdio.h>
#include "modules/core/dma.h"
#include "hardware/dma.h"
#include "hardware/adc.h"

// Inicializar ADC com DMA
bool dma_adc_init(dma_adc_config_t *config)
{
    config->dma_channel = get_dma_channel();
    if (config->dma_channel == -1)
    {
        printf("falou ligar dma");
        return false;
    }
    printf("deu certo ligar");
    config->dma_cfg = dma_channel_get_default_config(config->dma_channel);
    channel_config_set_transfer_data_size(&config->dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&config->dma_cfg, false);
    channel_config_set_write_increment(&config->dma_cfg, true);
    channel_config_set_dreq(&config->dma_cfg, DREQ_ADC);
    channel_config_set_ring(&config->dma_cfg, false, 0);
    return true;
}

// Pegar canal DMA
int get_dma_channel(void)
{
    int chan = dma_claim_unused_channel(true);
    return chan;
}

// Começar transferência de dados pelo DMA
void dma_adc_start_transfer(dma_adc_config_t *config, uint16_t *buffer, size_t sample_count)
{
    dma_channel_configure(
        config->dma_channel,
        &config->dma_cfg,
        buffer,
        &adc_hw->fifo,
        sample_count,
        true);
}

void dma_adc_wait_for_finish(dma_adc_config_t *config)
{
    dma_channel_wait_for_finish_blocking(config->dma_channel);
}

void dma_abort(uint dma_channel_to_abort)
{
    dma_channel_abort(dma_channel_to_abort);
}

void stop_adc_and_dma(uint dma_channel)
{
    adc_run(false);
    dma_abort(dma_channel);
}