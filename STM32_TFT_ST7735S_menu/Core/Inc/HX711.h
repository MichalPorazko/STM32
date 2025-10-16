
#pragma once

#include "main.h"

#include <stdint.h>

#define HX711_MAX_CAPTURE_BITS     32U


typedef struct
{
  long       	offset;
  float         scale;
  uint8_t       bit_buffer[HX711_MAX_CAPTURE_BITS];
  volatile uint8_t write_index;
  volatile uint8_t buffer_ready;
  uint8_t       buffer_length;
  uint8_t       read_index;
  GPIO_TypeDef  *clk_gpio;
  GPIO_TypeDef  *data_gpio;
  uint16_t      clk_pin;
  uint16_t      data_pin;
} hx711_t;


void hx711_init(hx711_t *hx711, GPIO_TypeDef *clk_gpio, uint16_t clk_pin, GPIO_TypeDef *data_gpio, uint16_t data_pin);
void hx711_timer1_PWM_low_callback(void);
void set_offset(hx711_t *hx711, long offset);


void tare(hx711_t *hx711, uint8_t times);
float get_weight(hx711_t *hx711, int8_t times);
