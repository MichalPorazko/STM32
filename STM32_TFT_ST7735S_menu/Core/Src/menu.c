#include "menu.h"
#include "lcd.h"

static void to_page1(void);
static void to_page2(void);

volatile uint32_t push_counter;

static MenuOption page1_options[] = {
    { L"START POMIARU", to_page2 },
};

static MenuOption page2_options[] = {
    { L"STOP POMIARU", to_page1 },
    { L"PRZERWIJ POMIAR", to_page1 },
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

static MenuPage *current_page = &page1;

static void to_page1(void) {
	current_page = &page1;
	menu_draw(current_page->option_count, current_page->selected, current_page->options->label);
}
static void to_page2(void) {
	current_page = &page2;
	menu_draw(current_page->option_count, current_page->selected, current_page->options->label);
}

void menu_init(void)
{
	lcd_init();
    current_page = &page1;
    page1.selected = 0;
    page2.selected = 0;
    menu_draw(current_page->option_count, current_page->selected, current_page->options->label);
}

/*

 	page1
		1, // option_count
		0  // selected

	page2
		2, // option_count
		0  // selected

	current_page = &page1;

	FOR PAGE1

	menu_next
	IF current_page->selected++;
		page1->selected == 1
		page1->option_count == 1

		page1->selected >= page1->option_count YES
		page1->selected = 0;


	menu_prev
	IF (page1->selected == 0
		page1->selected = page1->option_count - 1 = 1 - 1 = 0


 */

void menu_next(void)
{
    current_page->selected++;
    if (current_page->selected >= current_page->option_count) {
        current_page->selected = 0;
    }
    menu_draw(current_page->option_count, current_page->selected, current_page->options->label);
}

void menu_prev(void)
{
    if (current_page->selected == 0) {
        current_page->selected = current_page->option_count - 1;
    } else {
        current_page->selected--;
    }
    menu_draw(current_page->option_count, current_page->selected, current_page->options->label);
}


/*
 	 FOR PAGE1 IN CASE FOR CLICKING THE DOWN BUTTON


  	MenuOption *option = &current_page->options[current_page->selected] =
  	 	 	 	 	 	 page1->{ L"START POMIARU", to_page2 }[page1->0] =
  	 	 	 	 	 	 	 { L"START POMIARU", to_page2 }
  	option->callback = option->to_page2();

 */

void menu_select(void)
{
    MenuOption *option = &current_page->options[current_page->selected];
    if (option->callback) {
        option->callback();
    }
}

