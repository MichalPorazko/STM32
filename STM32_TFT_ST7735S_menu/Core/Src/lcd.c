

#include "lcd.h"
#include "spi.h"
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include "fontx.h"
#include "menu.h"


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

static void lcd_wait_for_transfer(void)
{
        while (lcd_is_busy()) {

        }
}

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
	lcd_wait_for_transfer();

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

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		lcd_transfer_done();
	}
}

static uint8_t font_height(const unsigned char *font)
{
    fontx_meta_t meta;
    if (0 != fontx_meta(&meta, font)) {
        return 0;
    }

    return meta.height;
}

static uint16_t text_width(const wchar_t *text, const unsigned char *font){

	uint16_t width = 0;
	    fontx_glyph_t glyph;
	    while (*text != L'\0') {
	        if (0 == fontx_glyph(&glyph, *text, font)) {
	            width = (uint16_t)(width + glyph.width);
	        }
	        text++;
	    }

	    return width;

}

void menu_draw(const MenuPage *page)
{
	hagl_clear_screen();

	const color_t gold = rgb565(255, 215, 0);
	const color_t red = rgb565(255, 0, 0);
	const color_t green = rgb565(0, 255, 0);
	const color_t white = rgb565(255, 255, 255);

	const int top_margin = 8;
	const int option_spacing = 12;
	const int horizontal_margin = 6;
	const int base_horizontal_padding = 12;
	const int vertical_padding = 4;

	uint8_t line_height = font_height(font6x9);
	if (line_height == 0U) {
		line_height = 9U;
	}
	const uint16_t rect_height = (uint16_t)(line_height + (uint8_t)(vertical_padding * 2));

	int y = top_margin;

	if (page->type == MENU_PAGE_START || page->type == MENU_PAGE_PAUSE) {
		wchar_t measurement_text[32];
		float measurement_value = menu_get_measurement_value();


//		if (measurement_value < 0.0f) {
//			measurement_value = 0.0f;
//		}
//		if (measurement_value > 1000.0f) {
//			measurement_value = 1000.0f;
//		}


//		uint16_t rounded_value = (uint16_t)(measurement_value + 0.5f);
		swprintf(measurement_text, sizeof(measurement_text) / sizeof(*measurement_text), L"%u ml", (unsigned int)measurement_value);
		uint16_t measurement_width = text_width(measurement_text, font6x9);
		int measurement_x = (LCD_WIDTH - measurement_width) / 2;
		hagl_put_text(measurement_text, measurement_x, y, white, font6x9);
		y += (int)line_height + option_spacing;
	} else {
		y += option_spacing;
	}

	for (uint8_t i = 0; i < page->option_count; i++) {
		const MenuOption *option = &page->options[i];
		uint16_t label_width = text_width(option->label, font6x9);
		uint16_t desired_width = (uint16_t)(label_width + (uint16_t)(base_horizontal_padding * 2));
		const uint16_t max_rect_width = (uint16_t)(LCD_WIDTH - (horizontal_margin * 2));
		uint16_t rect_width = desired_width;
		if (rect_width > max_rect_width) {
			rect_width = max_rect_width;
		}
		int rect_x = (LCD_WIDTH - rect_width) / 2;
		int text_x = rect_x + (rect_width - (int)label_width) / 2;
		int text_y = y + ((int)rect_height - (int)line_height) / 2;

		color_t color = (i == page->selected) ? green : red;
		hagl_fill_rectangle(rect_x, y, rect_x + rect_width - 1, y + rect_height - 1, color);
		hagl_draw_rectangle(rect_x, y, rect_x + rect_width - 1, y + rect_height - 1, gold);
		hagl_put_text(option->label, text_x, text_y, white, font6x9);

		y += rect_height + option_spacing;
	}

    lcd_copy();
}



