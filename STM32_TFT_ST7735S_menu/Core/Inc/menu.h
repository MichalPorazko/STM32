#pragma once

#include <stdint.h>
#include <wchar.h>
#include "lcd.h"
#include "main.h"
#include "HX711.h"

volatile uint16_t pin_debounce;

typedef void (*MenuCallback)(void);

typedef struct MenuOption {
        const wchar_t * const label;
    MenuCallback callback;
} MenuOption;


typedef struct {
    MenuOption *options;
    uint8_t option_count;
    uint8_t selected;
} MenuPage;

void menu_init(void);
void turn_off(void);
void menu_next(void);
void menu_prev(void);
void menu_select(void);

void button_debounce(uint16_t GPIO_Pin);

