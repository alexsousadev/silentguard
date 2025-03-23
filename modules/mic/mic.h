#include <stdint.h>
#include <math.h>

// Definições de constantes
#define MIC_CHANNEL 2
#define MIC_PIN 28
#define SAMPLES 100

#define ADC_MAX 4095
#define VREF 3.3
#define OFFSET 2048
#define SENSITIVITY 0.00631 // VRMS/Pa do microfone
#define GAIN 20

// Prototipo das funções
void init_mic(void);
void sample_mic(void);
float calculate_rms(const uint16_t *adc_buffer, uint32_t num_samples, float offset, float vref, float adc_max);
float adc_to_db(float vrms, float gain, float sensitivity);
float mic_get_db(void);