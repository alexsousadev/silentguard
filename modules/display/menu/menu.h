
#include <stdio.h>
#include <pico/stdlib.h>

extern int all_sample_count;
extern int all_sum_db;

// Prototipos das funções
void all_options_config(void);
void select_option_config(bool option);
void handle_automatic_config();
void handle_manual_config();
void menu_manual_config(void);
bool debounce(uint gpio);
