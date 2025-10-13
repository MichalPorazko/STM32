
#pragma once

#include "main.h"

#include <stdint.h>

#define HX711_DATA_BITS            24U
#define HX711_MAX_CAPTURE_BITS     32U

#define CHANNEL_A                  0U
#define CHANNEL_B                  1U




typedef struct
{
  long       	Aoffset;
  float         Ascale;
  uint8_t       Again;
  long          Boffset;
  float         Bscale;
  uint8_t       Bgain;
  uint8_t       bit_buffer[HX711_MAX_CAPTURE_BITS];
  volatile uint8_t write_index;
  volatile uint8_t buffer_ready;
  uint8_t       buffer_length;
  uint8_t       read_index;
} hx711_t;


void hx711_init(hx711_t *hx711, GPIO_TypeDef *clk_gpio, uint16_t clk_pin, GPIO_TypeDef *dat_gpio, uint16_t dat_pin);
void hx711_timer1_PWM_low_callback(void);
void set_offset(hx711_t *hx711, long offset, uint8_t channel);
long transform_reading(hx711_t *hx711, uint8_t channel);
long read_average(hx711_t *hx711, int8_t times, uint8_t channel);
double get_value(hx711_t *hx711, int8_t times, uint8_t channel);
void tare(hx711_t *hx711, uint8_t times, uint8_t channel);
void tare_all(hx711_t *hx711, uint8_t times);
float get_weight(hx711_t *hx711, int8_t times, uint8_t channel);
