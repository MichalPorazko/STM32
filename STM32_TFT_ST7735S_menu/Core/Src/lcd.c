

#include "lcd.h"
#include "spi.h"
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"

#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

#define LCD_OFFSET_X  1
#define LCD_OFFSET_Y  2



static uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];


#define CMD(x)			((x) | 0x100)


static const uint16_t init_table[] = {
  CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
  CMD(ST7735S_INVCTR), 0x07,
  CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
  CMD(ST7735S_PWCTR2), 0xc5,
  CMD(ST7735S_PWCTR3), 0x0a, 0x00,
  CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
  CMD(ST7735S_PWCTR5), 0x8a, 0xee,
  CMD(ST7735S_VMCTR1), 0x0e,
  CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
  CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
  CMD(0xf0), 0x01,
  CMD(0xf6), 0x00,
  CMD(ST7735S_COLMOD), 0x05,
  CMD(ST7735S_MADCTL), 0xa0,
};


/*
 Declaring a function as static which results that this function
  won't be visible by other files

 Whats so on, because it is static, a declaration of it is required
  to be initialized in a header file
 */

static void lcd_cmd(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_data(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}




static void lcd_send(uint16_t value)
{
	if (value & 0x100) {
		lcd_cmd(value);
	} else {
		lcd_data(value);
	}
}

static void lcd_data16(uint16_t value)
{
	lcd_data(value >> 8);
	lcd_data(value);
}


static void lcd_set_window(int x, int y, int width, int height)
{
	/*
	 if x is 10 and width is 5, the ending column should be 14 (10 + 5 - 1).
	 This is because the width includes the starting pixel itself. If we didn't subtract 1,
	 the ending column would be 15, which would include an extra pixel.
	 */

	lcd_cmd(ST7735S_CASET);
	lcd_data16(LCD_OFFSET_X + x);
	lcd_data16(LCD_OFFSET_X + x + width - 1);

	lcd_cmd(ST7735S_RASET);
	lcd_data16(LCD_OFFSET_Y + y);
	lcd_data16(LCD_OFFSET_Y + y + height- 1);
}


void lcd_init(void)
{

  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  int i;

  for (i = 0;  i < (sizeof(init_table) / sizeof(init_table[0])); i++){
	  lcd_send(init_table[i]);
  }


  HAL_Delay(200);
  lcd_cmd(ST7735S_SLPOUT);
  HAL_Delay(120);
  lcd_cmd(ST7735S_DISPON);
}




void lcd_fill_box(int x, int y, int width, int height, uint16_t color)
{
	lcd_set_window(x, y, width, height);

	lcd_cmd(ST7735S_RAMWR);
	for (int i = 0; i < width * height; i++)
		lcd_data16(color);
}


/*
 In a 2D grid (like a display screen), we have rows and columns.
 To represent this 2D grid in a 1D array (like our frame buffer),
  we need a way to map the 2D coordinates (x, y) to a 1D index.

  Verifying with an Example:
	Let's say we have a smaller display for simplicity: width = 4, height = 3.

	Frame buffer indices would look like this (row-major order):

	LCD_WIDTH = 4
	LCD_HEIGHT = 3

	Ekran:
	+----+----+----+----+
	|  0 |  1 |  2 |  3 |  ← y = 0
	+----+----+----+----+
	|  4 |  5 |  6 |  7 |  ← y = 1
	+----+----+----+----+
	|  8 |  9 | 10 | 11 |  ← y = 2
	+----+----+----+----+


	For a pixel at (x, y) = (2, 1): (assuming y=0 is the top row)

	y = 1 → starting index of row 1 is 1 * 4 = 4
	x = 2 → index = 4 + 2 = 6
	Which matches the position in the 2D grid.



 */


void lcd_put_pixel(int x, int y, uint16_t color)
{
	frame_buffer[x + y * LCD_WIDTH] = color;
}

void lcd_copy(void)
{
	lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
	lcd_cmd(ST7735S_RAMWR);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*)frame_buffer, sizeof(frame_buffer));
}


void lcd_transfer_done(void)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

bool lcd_is_busy(void)
{
	if (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_BUSY)
		return true;
	else
		return false;
}



void menu_draw(uint8_t option_count, uint8_t selected, const wchar_t * const label)
{
    color_t gold = rgb565(255, 215, 0);
    color_t red = rgb565(255, 0, 0);
    color_t green = rgb565(0, 255, 0);
    color_t white = rgb565(255, 255, 255);

    int rect_width = 80;
    int rect_height = 30;
    int spacing = 10;
    int x = (LCD_WIDTH - rect_width) / 2;
    int y = 20;

    hagl_clear_screen();
    hagl_draw_rectangle(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, gold);

    for (uint8_t i = 0; i < option_count; i++) {
        color_t color = (i == selected ) ? green : red;
        hagl_fill_rectangle(x, y, x + rect_width - 1, y + rect_height - 1, color);
        hagl_put_text(label, x + 5, y + 10, white, font6x9);
        y += rect_height + spacing;
    }

    lcd_copy();
}




