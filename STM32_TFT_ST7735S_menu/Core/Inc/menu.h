#pragma once

#include <stdint.h>
#include <wchar.h>
#include "main.h"
#include "HX711.h"

volatile uint16_t pin_debounce;

typedef void (*MenuCallback)(void);

typedef struct MenuOption {
        const wchar_t * const label;
    MenuCallback callback;
} MenuOption;

typedef enum {
    MENU_PAGE_MENU,
    MENU_PAGE_START,
    MENU_PAGE_PAUSE
} MenuPageType;


typedef struct MenuPage MenuPage;
struct MenuPage{
	MenuPageType type;
    MenuOption *options;
    uint8_t option_count;
    uint8_t selected;
};

void menu_init(void);
void turn_off(void);
void menu_next(void);
void menu_prev(void);
void menu_select(void);

const MenuPage *menu_current_page(void);
void menu_refresh(void);
void menu_set_measurement_value(float value_ml);
float menu_get_measurement_value(void);

void button_debounce(uint16_t GPIO_Pin);

