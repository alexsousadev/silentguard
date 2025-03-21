#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"

// Define a estrutura para armazenar a configuração do DMA e o canal
typedef struct dma_adc_config
{
    uint dma_channel;
    dma_channel_config dma_cfg;
} dma_adc_config_t;

bool dma_adc_init(dma_adc_config_t *config);
void dma_adc_start_transfer(dma_adc_config_t *config, uint16_t *buffer, size_t sample_count);
void dma_adc_wait_for_finish(dma_adc_config_t *config);
void dma_abort(uint dma_channel_to_abort);
void stop_adc_and_dma(uint dma_channel);
int get_dma_channel(void);
