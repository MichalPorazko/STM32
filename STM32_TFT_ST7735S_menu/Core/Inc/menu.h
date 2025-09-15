#pragma once

#include <stdint.h>
#include <wchar.h>

typedef void (*MenuCallback)(void);

typedef struct {
	const wchar_t * const label;
    MenuCallback callback;
} MenuOption;

typedef struct {
    MenuOption *options;
    uint8_t option_count;
    uint8_t selected;
} MenuPage;

void menu_init(void);
