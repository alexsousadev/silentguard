#include <stdio.h>
#include <pico/stdlib.h>

// Definições das variaveis para calibração
typedef struct
{
    int all_sample_count;
    int all_sum_db;
} CalibrationConfig;

extern CalibrationConfig calibration_config;

// Prototipos das funções
void all_options_config(void);
void select_option_config(bool option);
void handle_automatic_config();
void handle_manual_config();
void menu_manual_config(void);
bool debounce(uint gpio);
