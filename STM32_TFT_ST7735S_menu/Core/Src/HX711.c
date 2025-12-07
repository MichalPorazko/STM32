


#include "HX711.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "stdbool.h"
#include "menu.h"
#include "rtc.h"


hx711_t *active_hx711 = NULL;
static volatile uint8_t tim2_needs_rearm = 0U;

static long transform_reading( hx711_t *hx711);

void reset_parameters( hx711_t *hx711){

	hx711->critical = 0U;
	hx711->write_index = 0U;
	hx711->measurement_count = 0U;
	hx711->raw_reading = 0;
	hx711->start_measurement = 0U;

	hx711->sum = 0.0f;


}


void hx711_init( hx711_t *hx711, GPIO_TypeDef *data_gpio, uint16_t data_pin, GPIO_TypeDef *clk_gpio, uint16_t clk_pin)
{


  reset_parameters(hx711);
  hx711->data_gpio = data_gpio;
  hx711->data_pin = data_pin;

  hx711->sclk_pulses = htim1.Init.RepetitionCounter + 1U;

  hx711->clk_gpio = clk_gpio;
  hx711->clk_pin = clk_pin;

  hx711->scale = -1.017;
  hx711->new_patient = 1U;
  hx711->offset = -39878.57;

  hx711->processed_reading = 0.0f;

  active_hx711 = hx711;
}


void start_measurement(hx711_t *hx711){

	if (HAL_TIM_OnePulse_Start_IT(&htim1, TIM_CHANNEL_1) != HAL_OK)
	    {
	      Error_Handler();
	    }

}

void hx711_timer1_PWM_low_callback( hx711_t *hx711){

  uint8_t index = hx711->write_index;
  if (index < hx711->sclk_pulses)
  {
	  hx711->bit_buffer[index] = (HAL_GPIO_ReadPin(hx711->data_gpio, hx711->data_pin) ? 1U : 0U);
    index++;
    hx711->write_index = index;
  }
  else
  {
	  hx711->write_index = 0U;
  }

}

void hx711_update_reading( hx711_t *hx711)
{
  long raw_value = transform_reading(hx711);
  hx711->raw_reading = raw_value;


  double adjusted = (double)raw_value - (double)hx711->offset;
  hx711->processed_reading = (float)((hx711->processed_reading + (adjusted / (double)hx711->scale))/hx711->measurement_count);
}


static long transform_reading( hx711_t *hx711){

	unsigned long value = 0;
	uint8_t filler = 0x00;
	uint8_t data[3] = {0};

	for (uint8_t i = 0; i < 8; ++i) {
	        data[0] |= (hx711->bit_buffer[i] & 0x01U) << (7-i);
	        data[1] |= (hx711->bit_buffer[8+i] & 0x01U) << (7-i);
	        data[2] |= (hx711->bit_buffer[16+i] & 0x01U) << (7-i);
	}


	//binary 0x80 10000000
	if (data[2] & 0x80) {
		//11111111
		filler = 0xFF;
	} else {
		//00000000
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( (unsigned long)(filler) << 24
			| (unsigned long)(data[0]) << 16
			| (unsigned long)(data[1]) << 8
			| (unsigned long)(data[2]) );

	return (long)(value);
}

void pack_data( hx711_t *hx711) {

    // to keep track of the current position in the array
    size_t offset = 0U;

    /*

	"Stop bit and parity bits (if enabled) also contribute to the number of edges.

	For example, letter 'a' with ASCII code 97 is encoded as 0100001101 on the wire
	(with 8n1 configuration), start and stop bits included. This sequence has 3
	positive edges (transitions from 0 to 1). Therefore, to wake up the system
	when 'a' is sent, set wakeup_threshold=3. " -> configuration for esp32


     */

    memcpy((uint8_t *)hx711->tx_buffer + offset, "a", sizeof(char));
    offset += sizeof(char);

    float processed_reading = hx711->processed_reading;
    memcpy((uint8_t *)hx711->tx_buffer + offset, &processed_reading, sizeof(float));
    offset += sizeof(float);

    uint8_t critical = hx711->critical;
    memcpy((uint8_t *)hx711->tx_buffer + offset, &critical, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    uint8_t new_patient = hx711->new_patient;
    memcpy((uint8_t *)hx711->tx_buffer + offset, &new_patient, sizeof(uint8_t));
    offset += sizeof(uint8_t);


}

void HAL_TIM_PWM_PulseFinishedCallback (TIM_HandleTypeDef * htim){

	hx711_update_reading(active_hx711);
	active_hx711->measurement_count = (uint8_t)(active_hx711->measurement_count + 1U);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1) {

		pack_data(active_hx711);

		if (HAL_UART_Transmit_IT(&huart2, (uint8_t *)( active_hx711->tx_buffer), HX711_TX_BUFFER_SIZE) == HAL_OK){
			active_hx711->tx_in_progress = 1U;
			active_hx711->measurement_count = 0U;
		}
	}


    if (htim->Instance == TIM6) {
		HAL_TIM_Base_Stop_IT(&htim6);
		__HAL_TIM_SET_COUNTER(&htim6, 0);

		if (pin_debounce == BTN_UP_Pin)
			{
				menu_next();
				pin_debounce = 0;
			}
		if (pin_debounce == BTN_DOWN_Pin)
			{
				menu_prev();
				pin_debounce = 0;
			}
		if (pin_debounce == BTN_ENTER_Pin)
			{
				menu_select();
				pin_debounce = 0;
			}
		if (pin_debounce == BTN_POWER_Pin)
			{
				turn_off();
				pin_debounce = 0;
			}

    }
}


void pause_measurement(hx711_t *hx711){

	reset_parameters(hx711);
	disable_wakeup();

}

void end_measurement(hx711_t *hx711){

	pause_measurement(hx711);
	hx711->new_patient = 1U;
	hx711->processed_reading = 0;
}



