#include <stdio.h>
#include <stdint.h>

// Função para converter valores ARGB para RGB
void convert_to_rgb(uint32_t argb, int rgb[3])
{
    rgb[0] = argb & 0xFF;         // Blue
    rgb[1] = (argb >> 8) & 0xFF;  // Green
    rgb[2] = (argb >> 16) & 0xFF; // Red
}

// Função para verificar se um pixel é preto (para não acender)
int is_black(int rgb[3])
{
    if (rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 0)
    {
        return 1; // Se for preto, retorna 1 (desligado)
    }
    return 0; // Caso contrário, retorna 0 (ligado)
}

// Converte a matriz de pixels do piskel para a matriz de pixels RGB
void convert_piskel_to_matriz(uint32_t piskel_matriz[1][25], int pixel_draw[5][5], int pixel_rgb[5][5][3])
{
    // Array de cores e sinalização se o LED está ligado
    int rgb[3];
    int led_on = 0;

    // Percorre os pixels convertendo
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            uint32_t pixel = piskel_matriz[0][i * 5 + j]; // Acessa corretamente o pixel
            convert_to_rgb(pixel, rgb);                   // Converte para RGB

            // Verifica se o pixel é preto
            if (is_black(rgb))
            {
                led_on = 0; // Desliga o LED
            }
            else
            {
                led_on = 1; // Liga o LED
            }

            pixel_draw[i][j] = led_on; // Armazena o estado do LED na matriz

            // Armazena as cores RGB na matriz
            for (int k = 0; k < 3; k++)
            {
                pixel_rgb[i][j][k] = rgb[k];
            }
        }
    }
}
