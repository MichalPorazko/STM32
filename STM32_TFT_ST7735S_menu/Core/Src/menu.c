#include "menu.h"
#include "tim.h"

static void to_page1(void);
static void to_page2(void);
static void to_page3(void);

static void start_resume_measurement_option(void);
static void pause_measurement_option(void);
static void end_measurement_option(void);




volatile uint16_t pin_debounce;


static MenuOption page1_options[] = {
    { L"START POMIARU", start_resume_measurement_option }
};

static MenuOption page2_options[] = {
	{ L"PRZERWIJ POMIAR", pause_measurement_option },
	{ L"KONIEC POMIARU", end_measurement_option }

};

static MenuOption page3_options[] = {
	{ L"WZNOW POMIAR", start_resume_measurement_option },
	{ L"KONIEC POMIARU", end_measurement_option }
};

static MenuPage page1 = {
		page1_options,
		1, // option_count
		0  // selected
};

static MenuPage page2 = {
		page2_options,
		2, // option_count
		0  // selected
};


static MenuPage page3 = {
		page3_options,
		2, // option_count
		0  // selected
};



static MenuPage *current_page = &page1;

static void to_page1(void) {
	current_page = &page1;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void to_page2(void) {
	current_page = &page2;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void to_page3(void) {
	current_page = &page3;
	menu_draw(current_page->option_count, current_page->selected, current_page->options);
}

static void start_measurement_option(void) {
	to_page2();
	active_hx711->start_measurement = 1U;
}

static void pause_measurement_option(void) {
	to_page3();
	pause_measurement();
}

static void start_resume_measurement_option(void) {
	to_page2();
	active_hx711->start_measurement = 1U;
}

static void end_measurement_option(void) {
	to_page1();
	end_measurement();
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
