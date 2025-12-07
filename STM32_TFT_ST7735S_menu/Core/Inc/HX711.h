
#pragma once

#include "main.h"
#include "tim.h"
#include "usart.h"

#include <stdint.h>

#define HX711_TX_BUFFER_SIZE 56U
#define HX711_BUFFER_SIZE 32U
#define measurement_threshold	   10U


typedef struct
{

	uint8_t		start_measurement;
	long       	offset;
	float         scale;
	uint8_t       bit_buffer[HX711_BUFFER_SIZE];
	volatile uint8_t write_index;

	uint8_t sclk_pulses;
	uint8_t       read_index;
	GPIO_TypeDef  *data_gpio;
	uint16_t      data_pin;

	GPIO_TypeDef  *clk_gpio;
	uint16_t      clk_pin;

	uint8_t       measurement_count;
	uint8_t       tx_buffer[HX711_TX_BUFFER_SIZE];
	uint8_t       tx_in_progress;

	uint8_t new_patient;
	uint8_t critical;


	long          raw_reading;
	float         processed_reading;
	float sum;

} hx711_t;

void hx711_init( hx711_t *hx711, GPIO_TypeDef *data_gpio, uint16_t data_pin,
		GPIO_TypeDef *clk_gpio, uint16_t clk_pin);
void start_measurement(hx711_t *hx711);
void hx711_timer1_PWM_low_callback(hx711_t* hx711);
void hx711_update_reading(hx711_t *hx711);
void pause_measurement(hx711_t *hx711);
void end_measurement(hx711_t *hx711);


void pack_data(hx711_t* hx711);

extern  hx711_t* active_hx711;
