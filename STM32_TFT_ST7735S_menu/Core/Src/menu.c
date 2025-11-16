#include "menu.h"
#include "tim.h"
#include "lcd.h"

static void to_menu_page(void);
static void to_start_page(void);
static void to_pause_page(void);

static void start_resume_measurement_option(void);
static void pause_measurement_option(void);
static void end_measurement_option(void);





volatile uint16_t pin_debounce;



static MenuOption menu_page_options[] = {
    { L"START POMIARU", start_resume_measurement_option }
};

static MenuOption start_page_options[] = {
	{ L"PRZERWIJ POMIAR", pause_measurement_option },
	{ L"KONIEC POMIARU", end_measurement_option }

};

static MenuOption pause_page_options[] = {
	{ L"WZNOW POMIAR", start_resume_measurement_option },
	{ L"KONIEC POMIARU", end_measurement_option }
};



static MenuPage menu_page = {
		MENU_PAGE_MENU,
		menu_page_options,
		1, // option_count
		0  // selected
};

static MenuPage start_page = {
		MENU_PAGE_START,
		start_page_options,
		2, // option_count
		0  // selected
};


static MenuPage pause_page = {
		MENU_PAGE_PAUSE,
		pause_page_options,
		2, // option_count
		0  // selected
};




static MenuPage *current_page = &menu_page;

static void to_menu_page(void) {
	current_page = &menu_page;
	menu_draw(current_page);
}

static void to_start_page(void) {
	current_page = &start_page;
	menu_draw(current_page);
}

static void to_pause_page(void) {


	current_page = &pause_page;
	menu_draw(current_page);
}



static void pause_measurement_option(void) {
	pause_measurement();
	to_pause_page();
}

static void start_resume_measurement_option(void) {
	to_start_page();
	active_hx711->start_measurement = 1U;

}

static void end_measurement_option(void) {
	to_menu_page();
	end_measurement();
}




void menu_init(void)
{
	lcd_init();
    current_page = &menu_page;
    menu_page.selected = 0;
    start_page.selected = 0;
    pause_page.selected = 0; //if this needed???
    menu_draw(current_page);
}

void menu_next(void)
{
    current_page->selected++;
    if (current_page->selected >= current_page->option_count) {
        current_page->selected = 0;
    }
    menu_draw(current_page);
}

void menu_prev(void)
{
    if (current_page->selected == 0) {
        current_page->selected = current_page->option_count - 1;
    } else {
        current_page->selected--;
    }
    menu_draw(current_page);
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

void menu_refresh(void){
	menu_draw(current_page);

}


float menu_get_measurement_value(void)
{
	return active_hx711->processed_reading;
}


void button_debounce(uint16_t GPIO_Pin) {

        __HAL_TIM_SET_COUNTER(&htim6, 0);
        HAL_TIM_Base_Start_IT(&htim6);
        pin_debounce = GPIO_Pin;


}
