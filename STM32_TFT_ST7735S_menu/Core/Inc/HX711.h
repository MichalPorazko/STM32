
#pragma once

#include "main.h"
#include "tim.h"
#include "usart.h"

#include <stdint.h>

#define HX711_TX_BUFFER_SIZE 56U

#define measurement_threshold	   10U
#define SCLK_pulses					27U


typedef struct
{

	uint8_t		start_measurement;
	long       	offset;
	float         scale;
	volatile uint8_t write_index;

	uint8_t       buffer_length;
	uint8_t       read_index;
	uint8_t 		data_to_send;
	GPIO_TypeDef  *data_gpio;
	uint16_t      data_pin;

	GPIO_TypeDef  *clk_gpio;
	uint16_t      clk_pin;

	uint8_t       measurement_count;
	uint8_t       tx_buffer[HX711_TX_BUFFER_SIZE];
	uint8_t       tx_in_progress;

	uint8_t new_patient;
	uint8_t critical;

	uint8_t sclk_pulses;
	uint8_t gain;

	long          raw_reading;
	float         processed_reading;
	float sum;



} hx711_t;

void hx711_init( hx711_t *hx711, GPIO_TypeDef *data_gpio, uint16_t data_pin,
		GPIO_TypeDef *clk_gpio, uint16_t clk_pin);
void start_measurement(hx711_t *hx711);
void pause_measurement(hx711_t *hx711);
void end_measurement(hx711_t *hx711);


void pack_data(hx711_t* hx711);

extern  hx711_t* active_hx711;
