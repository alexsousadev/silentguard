#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "./convert_to_leds.h"
#include "delay.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

// Declaração das matrizes de pixels
int pixelDraw[5][5];
int pixelRGB[5][5][3];

// Definição de pixel GRB
struct pixel_t
{
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin)
{

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0)
  {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear()
{
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite()
{
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

int matrizLeds[5][5] = {
    {24, 23, 22, 21, 20}, // 24 23 22 21 20
    {15, 16, 17, 18, 19}, // 15 16 17 18 19
    {14, 13, 12, 11, 10}, // 14 13 12 11 10
    {5, 6, 7, 8, 9},      // 5 6 7 8 9
    {4, 3, 2, 1, 0}};     // 4 3 2 1 0

void matrizToLeds(int draw[5][5], int color[3])
{

  int matrizLeds[5][5] = {
      {24, 23, 22, 21, 20}, // 24 23 22 21 20
      {15, 16, 17, 18, 19}, // 15 16 17 18 19
      {14, 13, 12, 11, 10}, // 14 13 12 11 10
      {5, 6, 7, 8, 9},      // 5 6 7 8 9
      {4, 3, 2, 1, 0}};     // 4 3 2 1 0

  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if (draw[i][j] == 1)
      {
        npSetLED(matrizLeds[i][j], color[0], color[1], color[2]);
      }
    }
  }
}

// Converte
void convertMatrizToLed(int pixelDraw[5][5], int pixelRGB[5][5][3])
{
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if (pixelDraw[i][j] == 1) // Verifica se o LED está ligado
      {
        // Usa o índice correto da matriz de LEDs
        npSetLED(matrizLeds[i][j], pixelRGB[i][j][0], pixelRGB[i][j][1], pixelRGB[i][j][2]);
      }
    }
  }
}

// inicialização da matriz de LEDs
void init_matriz_leds()
{
  npInit(LED_PIN);
  npClear();
}

// desenha emoji feliz na matriz de leds
void emojiFeliz()
{
  npClear();
  static uint32_t new_piskel_data[1][25] = {
      {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0x00000000, 0xff793600, 0x00000000, 0xff793600, 0x00000000,
       0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0xff793600, 0x00000000, 0x00000000, 0x00000000, 0xff793600,
       0x00000000, 0xff793600, 0xff793600, 0xff793600, 0x00000000}};

  convertPiskelToMatriz(new_piskel_data, pixelDraw, pixelRGB);
  convertMatrizToLed(pixelDraw, pixelRGB);
  npWrite();
}

// desenha um emoji triste na matriz de leds
void emojiTriste()
{
  npClear();
  static uint32_t new_piskel_data[1][25] = {
      {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0x00000000, 0xff0000ff, 0x00000000, 0xff0000ff, 0x00000000,
       0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0x00000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0x00000000,
       0xff0000ff, 0x00000000, 0x00000000, 0x00000000, 0xff0000ff}};

  convertPiskelToMatriz(new_piskel_data, pixelDraw, pixelRGB);
  convertMatrizToLed(pixelDraw, pixelRGB);
  npWrite();
}