#include "matriz_leds.h"
#include "./convert_to_leds.h"
#include "ws2818b.pio.h"
#include "modules/core/pwm.h"

// Declaração das matrizes de pixels
int pixel_draw[5][5];
int pixel_rgb[5][5][3];

// Definição de pixel RGB
struct pixel_t
{
  uint8_t g, r, b;
};
typedef struct pixel_t pixel_t;
typedef pixel_t np_led_t;

// Declaração do buffer de pixels que formam a matriz.
np_led_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

// Inicializa a máquina PIO para controle da matriz de LEDs.
void np_init(uint pin)
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
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 0;
  }
}

//  Atribui uma cor RGB a um LED.
void np_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
  leds[index].r = r;
  leds[index].g = g;
  leds[index].b = b;
}

// Limpa o buffer de pixels.
void np_clear()
{
  for (uint i = 0; i < LED_COUNT; ++i)
    np_set_led(i, 0, 0, 0);
}

// Escreve os dados do buffer nos LEDs.
void np_write()
{
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].g);
    pio_sm_put_blocking(np_pio, sm, leds[i].r);
    pio_sm_put_blocking(np_pio, sm, leds[i].b);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

int matriz_leds[5][5] = {
    {24, 23, 22, 21, 20},
    {15, 16, 17, 18, 19},
    {14, 13, 12, 11, 10},
    {5, 6, 7, 8, 9},
    {4, 3, 2, 1, 0}};

void matriz_to_leds(int draw[5][5], int color[3])
{
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if (draw[i][j] == 1)
      {
        np_set_led(matriz_leds[i][j], color[0], color[1], color[2]);
      }
    }
  }
}

void convert_matriz_to_led(int pixel_draw[5][5], int pixel_rgb[5][5][3])
{
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if (pixel_draw[i][j] == 1)
      {
        np_set_led(matriz_leds[i][j], pixel_rgb[i][j][0], pixel_rgb[i][j][1], pixel_rgb[i][j][2]);
      }
    }
  }
}

// Inicialização da matriz de LEDs
void init_matriz_leds()
{
  np_init(LED_PIN);
  np_clear();
}

// Desenha um emoji feliz na matriz de LEDs
void happy_emoji()
{
  np_clear();
  static uint32_t new_piskel_data[1][25] = {
      {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0x00000000, 0xff793600, 0x00000000, 0xff793600, 0x00000000,
       0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0xff793600, 0x00000000, 0x00000000, 0x00000000, 0xff793600,
       0x00000000, 0xff793600, 0xff793600, 0xff793600, 0x00000000}};

  convert_piskel_to_matriz(new_piskel_data, pixel_draw, pixel_rgb);
  convert_matriz_to_led(pixel_draw, pixel_rgb);
  np_write();
}

// Desenha um emoji triste na matriz de LEDs
void sad_emoji()
{
  np_clear();
  static uint32_t new_piskel_data[1][25] = {
      {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0x00000000, 0xff0000ff, 0x00000000, 0xff0000ff, 0x00000000,
       0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
       0x00000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0x00000000,
       0xff0000ff, 0x00000000, 0x00000000, 0x00000000, 0xff0000ff}};

  convert_piskel_to_matriz(new_piskel_data, pixel_draw, pixel_rgb);
  convert_matriz_to_led(pixel_draw, pixel_rgb);
  np_write();
}
