#include "ssd1306.h"
#include "fonts.h"

// Inicializa a estrutura do display SSD1306
void ssd1306_init(ssd1306_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c)
{
  ssd->width = width;                                      // Largura do display (pixels)
  ssd->height = height;                                    // Altura do display (pixels)
  ssd->pages = height / 8U;                                // Páginas (cada página tem 8 pixels de altura)
  ssd->address = address;                                  // Endereço I2C do display
  ssd->i2c_port = i2c;                                     // Porta I2C para comunicação
  ssd->bufsize = ssd->pages * ssd->width + 1;              // Tamanho do buffer RAM
  ssd->ram_buffer = calloc(ssd->bufsize, sizeof(uint8_t)); // Aloca buffer RAM
  ssd->ram_buffer[0] = 0x40;                               // Byte de controle de dados (0x40)
  ssd->port_buffer[0] = 0x80;                              // Byte de controle de comandos (0x80)
}

// Configura o display SSD1306 com ajustes iniciais
void ssd1306_config(ssd1306_t *ssd)
{
  ssd1306_command(ssd, SET_DISP | 0x00);            // Desliga display (para configuração)
  ssd1306_command(ssd, SET_MEM_ADDR);               // Modo de endereçamento de memória
  ssd1306_command(ssd, 0x01);                       // 0x01 = Modo de endereçamento de página
  ssd1306_command(ssd, SET_DISP_START_LINE | 0x00); // Linha inicial de display = 0
  ssd1306_command(ssd, SET_SEG_REMAP | 0x01);       // Remapeamento de segmento
  ssd1306_command(ssd, SET_MUX_RATIO);              // Ratio de multiplexação (linhas)
  ssd1306_command(ssd, HEIGHT - 1);                 // Altura do display - 1
  ssd1306_command(ssd, SET_COM_OUT_DIR | 0x08);     // Direção de varredura COM
  ssd1306_command(ssd, SET_DISP_OFFSET);            // Offset do display
  ssd1306_command(ssd, 0x00);                       // Sem offset
  ssd1306_command(ssd, SET_COM_PIN_CFG);            // Configuração dos pinos COM
  ssd1306_command(ssd, 0x12);                       // Configuração específica para 128x64
  ssd1306_command(ssd, SET_DISP_CLK_DIV);           // Divisão do clock do display
  ssd1306_command(ssd, 0x80);                       // Valor recomendado para clock
  ssd1306_command(ssd, SET_PRECHARGE);              // Período de pre-carga
  ssd1306_command(ssd, 0xF1);                       // Fases 1 e 2 de pre-carga
  ssd1306_command(ssd, SET_VCOM_DESEL);             // Nível de deseleção VCOMH
  ssd1306_command(ssd, 0x30);                       // Nível VCOMH
  ssd1306_command(ssd, SET_CONTRAST);               // Controle de contraste
  ssd1306_command(ssd, 0xFF);                       // Contraste máximo
  ssd1306_command(ssd, SET_ENTIRE_ON);              // Display ON/OFF segue RAM
  ssd1306_command(ssd, SET_NORM_INV);               // Display normal (não invertido)
  ssd1306_command(ssd, SET_CHARGE_PUMP);            // Habilita regulador charge pump
  ssd1306_command(ssd, 0x14);                       // Habilita charge pump ao ligar display
  ssd1306_command(ssd, SET_DISP | 0x01);            // Liga display (após configuração)
}

// Envia um comando para o controlador SSD1306
void ssd1306_command(ssd1306_t *ssd, uint8_t command)
{
  ssd->port_buffer[1] = command; // Comando a ser enviado
  i2c_write_blocking(
      ssd->i2c_port,    // Porta I2C
      ssd->address,     // Endereço do display
      ssd->port_buffer, // Buffer com o comando
      2,                // Envia 2 bytes (controle + comando)
      false);           // Não manter o master ocupado
}

// Envia o buffer RAM para atualizar o display
void ssd1306_send_data(ssd1306_t *ssd)
{
  ssd1306_command(ssd, SET_COL_ADDR);   // Comando para definir endereço da coluna
  ssd1306_command(ssd, 0);              // Coluna inicial (0)
  ssd1306_command(ssd, ssd->width - 1); // Coluna final (largura do display)
  ssd1306_command(ssd, SET_PAGE_ADDR);  // Comando para definir endereço da página
  ssd1306_command(ssd, 0);              // Página inicial (0)
  ssd1306_command(ssd, ssd->pages - 1); // Página final (última página)
  i2c_write_blocking(
      ssd->i2c_port,   // Porta I2C
      ssd->address,    // Endereço do display
      ssd->ram_buffer, // Buffer RAM com dados dos pixels
      ssd->bufsize,    // Tamanho do buffer
      false);          // Não manter o master ocupado
}

// Define ou limpa um pixel no buffer RAM
void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool value)
{
  uint16_t index = (y >> 3) + (x << 3) + 1; // Calcula índice no buffer
  uint8_t pixel = (y & 0b111);              // Bit dentro do byte (0-7)
  if (value)
    ssd->ram_buffer[index] |= (1 << pixel); // Define bit para 1 (liga pixel)
  else
    ssd->ram_buffer[index] &= ~(1 << pixel); // Limpa bit para 0 (desliga pixel)
}

// Preenche o display com uma cor (preto ou branco)
void ssd1306_fill(ssd1306_t *ssd, bool value)
{
  for (uint8_t y = 0; y < ssd->height; ++y) // Itera por todas as linhas
  {
    for (uint8_t x = 0; x < ssd->width; ++x) // Itera por todas as colunas
    {
      ssd1306_pixel(ssd, x, y, value); // Define cada pixel com o valor
    }
  }
}

// Desenha um retângulo (vazio ou preenchido)
void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill)
{
  for (uint8_t x = left; x < left + width; ++x) // Desenha linhas horizontais
  {
    ssd1306_pixel(ssd, x, top, value);
    ssd1306_pixel(ssd, x, top + height - 1, value);
  }
  for (uint8_t y = top; y < top + height; ++y) // Desenha linhas verticais
  {
    ssd1306_pixel(ssd, left, y, value);
    ssd1306_pixel(ssd, left + width - 1, y, value);
  }

  if (fill) // Preenche o retângulo se 'fill' for verdadeiro
  {
    for (uint8_t x = left + 1; x < left + width - 1; ++x)
    {
      for (uint8_t y = top + 1; y < top + height - 1; ++y)
      {
        ssd1306_pixel(ssd, x, y, value); // Preenche interior do retângulo
      }
    }
  }
}

// Desenha uma linha entre (x0, y0) e (x1, y1) (algoritmo de Bresenham)
void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value)
{
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true)
  {
    ssd1306_pixel(ssd, x0, y0, value); // Desenha pixel atual
    if (x0 == x1 && y0 == y1)
      break;

    int e2 = err * 2;
    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

// Desenha linha horizontal de x0 a x1 em y
void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value)
{
  for (uint8_t x = x0; x <= x1; ++x)
    ssd1306_pixel(ssd, x, y, value); // Define pixels na linha
}

// Desenha linha vertical de y0 a y1 em x
void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value)
{
  for (uint8_t y = y0; y <= y1; ++y)
    ssd1306_pixel(ssd, x, y, value); // Define pixels na linha
}

// Desenha um caractere no display
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y)
{
  uint16_t index = 0;

  if (c >= 'A' && c <= 'Z')
    index = (c - 'A' + 11) * 8;
  else if (c >= '0' && c <= '9')
    index = (c - '0' + 1) * 8;
  else if (c >= 'a' && c <= 'z')
    index = (c - 'a' + 37) * 8;
  else if (c == ':')
    index = 63 * 8;
  else if (c == '-')
    index = 64 * 8;

  for (uint8_t i = 0; i < 8; ++i)
  {
    uint8_t line = font[index + i]; // Linha vertical do caractere
    for (uint8_t j = 0; j < 8; ++j)
    {
      ssd1306_pixel(ssd, x + i, y + j, line & (1 << j)); // Define pixels conforme fonte
    }
  }
}

// Desenha uma string no display
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y)
{
  while (*str) // Itera pela string
  {
    ssd1306_draw_char(ssd, *str++, x, y); // Desenha caractere
    x += 8;                               // Avança para a próxima coluna
    if (x + 8 >= ssd->width)
    {
      x = 0;
      y += 8;
    } // Quebra de linha
    if (y + 8 >= ssd->height)
      break; // Fim do display
  }
}

// Inicializa I2C, configura display, limpa display
void ssd1306_init_config_clean(ssd1306_t *ssd, uint SCL, uint SDA, i2c_inst_t *PORT, uint8_t address)
{
  i2c_init(PORT, 400 * 1000);            // Inicializa I2C a 400kHz
  gpio_set_function(SDA, GPIO_FUNC_I2C); // Define função do pino SDA para I2C
  gpio_set_function(SCL, GPIO_FUNC_I2C); // Define função do pino SCL para I2C
  gpio_pull_up(SDA);                     // Pull-up para SDA
  gpio_pull_up(SCL);                     // Pull-up para SCL

  ssd1306_init(ssd, WIDTH, HEIGHT, false, address, PORT); // Inicializa estrutura
  ssd1306_config(ssd);                                    // Configura display
  ssd1306_send_data(ssd);                                 // Envia dados iniciais

  ssd1306_fill(ssd, false); // Limpa display (preto)
  ssd1306_send_data(ssd);   // Atualiza display
}

// Desenha um bitmap no display
void ssd1306_draw_bitmap(ssd1306_t *ssd, const uint8_t *bitmap)
{
  for (int i = 0; i < ssd->bufsize - 1; i++)
  {
    ssd->ram_buffer[i + 1] = bitmap[i]; // Copia bitmap para buffer
  }
  ssd1306_send_data(ssd); // Atualiza display com bitmap
}

// Limpa a tela do display (preto)
void ssd1306_clear_screen(ssd1306_t *ssd)
{
  ssd1306_fill(ssd, false); // Preenche com preto
  ssd1306_send_data(ssd);   // Atualiza display
}
