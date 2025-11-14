#include "menu.h"
#include "tim.h"

static void to_menu_page(void);
static void to_start_page(void);
static void to_pause_page(void);
static void to_settings_page(void);

static void start_resume_measurement_option(void);
static void pause_measurement_option(void);
static void end_measurement_option(void);
static void settings(void);




volatile uint16_t pin_debounce;


static MenuOption menu_page_options[] = {
    { L"START POMIARU", start_resume_measurement_option },
	{ L"USTAWIENIA", settings }
};

static MenuOption start_page_options[] = {
	{ L"PRZERWIJ POMIAR", pause_measurement_option },
	{ L"KONIEC POMIARU", end_measurement_option }

};

static MenuOption pause_page_options[] = {
	{ L"WZNOW POMIAR", start_resume_measurement_option },
	{ L"KONIEC POMIARU", end_measurement_option },
	{ L"USTAWIENIA", settings }
};

static MenuOption settings_page_options[] = {
	{ L"Skalibruj", end_measurement_option },
	{ L"START POMIARU", start_resume_measurement_option }
};

static MenuPage menu_page = {
		menu_page_options,
		2, // option_count
		0  // selected
};

static MenuPage start_page = {
		start_page_options,
		2, // option_count
		0  // selected
};


static MenuPage pause_page = {
		pause_page_options,
		3, // option_count
		0  // selected
};

static MenuPage settings_page = {
		settings_page_options,
		2, // option_count
		0  // selected
};



static MenuPage *current_page = &menu_page;

static void to_menu_page(void) {
	current_page = &menu_page;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void to_start_page(void) {
	current_page = &start_page;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void to_pause_page(void) {
	current_page = &pause_page;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void to_settings_page(void) {
	current_page = &settings_page;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void start_measurement_option(void) {
	to_start_page();
	active_hx711->start_measurement = 1U;
}

static void pause_measurement_option(void) {
	to_pause_page();
	pause_measurement();
}

static void start_resume_measurement_option(void) {
	to_start_page();
	active_hx711->start_measurement = 1U;
}

static void end_measurement_option(void) {
	to_menu_page();
	end_measurement();
}

static void settings_option(void) {
	to_settings_page();
}



void menu_init(void)
{
	lcd_init();
    current_page = &page1;
    page1.selected = 0;
    page2.selected = 0;
    page3.selected = 0; //if this needed???
    menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

void menu_next(void)
{
    current_page->selected++;
    if (current_page->selected >= current_page->option_count) {
        current_page->selected = 0;
    }
    menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

void menu_prev(void)
{
    if (current_page->selected == 0) {
        current_page->selected = current_page->option_count - 1;
    } else {
        current_page->selected--;
    }
    menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

void menu_select(void)
{
    MenuOption *option = &current_page->options[current_page->selected];
    if (option->callback) {
        option->callback();
    }
}

void turn_off(void) {

	//turn off code

}


void button_debounce(uint16_t GPIO_Pin) {

        __HAL_TIM_SET_COUNTER(&htim6, 0);
        HAL_TIM_Base_Start_IT(&htim6);
        pin_debounce = GPIO_Pin;


}
