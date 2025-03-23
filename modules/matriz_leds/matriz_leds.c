#include "matriz_leds.h"
#include "ws2818b.pio.h"

//==============================================================================
// Estruturas e Definições
//==============================================================================

struct pixel_t
{
  uint8_t g, r, b;
};

typedef struct pixel_t pixel_t;
typedef pixel_t np_led_t;

// Declaração das variáveis globais
int pixel_draw[5][5];
int pixel_rgb[5][5][3];

// Declaração do buffer de LEDs
np_led_t leds[LED_COUNT];

// Definição de matriz de LEDs
int matriz_leds[5][5] = {
    {24, 23, 22, 21, 20},
    {15, 16, 17, 18, 19},
    {14, 13, 12, 11, 10},
    {5, 6, 7, 8, 9},
    {4, 3, 2, 1, 0}};

// Variáveis para a máquina PIO
PIO np_pio;
uint sm;

//==============================================================================
// Funções de Inicialização e Controle
//==============================================================================

// Inicializa a máquina PIO para controle da matriz de LEDs
void np_init(uint pin)
{
  // Cria programa PIO
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0)
  {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa o buffer de LEDs
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 0;
  }
}

// Limpa o buffer de LEDs
void np_clear()
{
  for (uint i = 0; i < LED_COUNT; ++i)
    np_set_led(i, 0, 0, 0);
}

// Atribui uma cor RGB a um LED
void np_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
  leds[index].r = r;
  leds[index].g = g;
  leds[index].b = b;
}

// Escreve os dados do buffer nos LEDs
void np_write()
{
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].g);
    pio_sm_put_blocking(np_pio, sm, leds[i].r);
    pio_sm_put_blocking(np_pio, sm, leds[i].b);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet
}

// Inicializa a matriz de LEDs
void init_matriz_leds()
{
  np_init(LED_PIN);
  np_clear();
}

//==============================================================================
// Funções de Conversão e Exibição
//==============================================================================

// Converter valores ARGB para RGB
void convert_to_rgb(uint32_t argb, int rgb[3])
{
  rgb[0] = argb & 0xFF;         // Blue
  rgb[1] = (argb >> 8) & 0xFF;  // Green
  rgb[2] = (argb >> 16) & 0xFF; // Red
}

// Verificar se um pixel é preto (para não acender)
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

// Converte a matriz de pixels para LEDs
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

//==============================================================================
// Desenhos na Matriz de LEDs (Emojis)
//==============================================================================

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
