

#pragma once
#include <stdint.h>
#include <stdbool.h>


#define LCD_WIDTH	160
#define LCD_HEIGHT	128

void lcd_init(void);

#define BLACK     0x0000
#define RED       0xf800
#define GREEN     0x07e0
#define BLUE      0x001f
#define YELLOW    0xffe0
#define MAGENTA   0xf81f
#define CYAN      0x07ff
#define WHITE     0xffff

void lcd_fill_box(int x, int y, int width, int height, uint16_t color);
void lcd_put_pixel(int x, int y, uint16_t color);
void lcd_copy(void);
void lcd_transfer_done(void);
bool lcd_is_busy(void);


