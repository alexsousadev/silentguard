#include "pico/time.h" // Inclui a biblioteca para manipulação de tempo no Raspberry Pi Pico

// Função para controlar intervalos de tempo com precisão
// void setInterval(int valueSleep)
// {
//     static absolute_time_t next_sample_time; // Variável estática que armazena o próximo tempo de amostragem
//     static bool first_call = true;           // Variável estática para verificar se é a primeira chamada da função

//     // Se for a primeira chamada da função
//     if (first_call)
//     {
//         next_sample_time = get_absolute_time(); // Define o tempo atual como o próximo tempo de amostragem
//         first_call = false;                     // Marca que a primeira chamada foi feita
//         return;                                 // Retorna imediatamente, sem fazer nada (apenas inicializa o tempo de amostragem)
//     }

//     // Calcula o próximo tempo de amostragem, atrasado pelo valor fornecido (valueSleep, em milissegundos)
//     next_sample_time = delayed_by_ms(next_sample_time, valueSleep);

//     // Verifica se o tempo atual já passou do próximo tempo de amostragem
//     // (ou seja, se a diferença entre o tempo atual e o próximo tempo de amostragem é menor ou igual a zero)
//     if (absolute_time_diff_us(get_absolute_time(), next_sample_time) <= 0)
//     {
//         // Se o tempo atual já passou, calcula o próximo tempo de amostragem com base no tempo atual
//         next_sample_time = delayed_by_ms(get_absolute_time(), valueSleep);
//     }

//     // Coloca o processador em "sleep" até o próximo tempo de amostragem
//     sleep_until(next_sample_time);
// }
