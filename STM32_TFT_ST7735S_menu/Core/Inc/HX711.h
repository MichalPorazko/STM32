
#pragma once

#include "main.h"
#include "tim.h"
#include "usart.h"

#include <stdint.h>

#define HX711_BUFFER_SIZE 32U
#define HX711_TX_BUFFER_SIZE (sizeof(float) + sizeof(uint8_t) + sizeof(uint8_t))

#define measurement_threshold	   10U


typedef struct
{
  long       	offset;
  float         scale;
  uint8_t       bit_buffer[HX711_BUFFER_SIZE];

  //does it needs to be volatile???
  volatile uint8_t write_index;

  uint8_t       buffer_length;
  uint8_t       read_index;
  uint8_t 		data_to_send;
  GPIO_TypeDef  *data_gpio;
  uint16_t      data_pin;

  uint8_t       measurement_count;
  uint8_t       tx_buffer[HX711_TX_BUFFER_SIZE];
  uint8_t       tx_length;
  uint8_t       tx_in_progress;

  uint8_t new_patient;
  uint8_t critical;

  long          raw_reading;
  float         processed_reading;

} hx711_t;

void hx711_init(volatile hx711_t* hx711, GPIO_TypeDef* data_gpio, uint16_t data_pin);
void start_measurement(void);
void hx711_timer1_PWM_low_callback(volatile hx711_t* hx711);
void hx711_update_reading(volatile hx711_t *hx711);

void pack_data(volatile hx711_t* hx711);

extern volatile hx711_t* active_hx711;
