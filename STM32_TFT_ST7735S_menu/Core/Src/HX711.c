


#include "HX711.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

volatile hx711_t *active_hx711 = NULL;

static long transform_reading(hx711_t *hx711);


void hx711_init(hx711_t *hx711, GPIO_TypeDef *data_gpio, uint16_t data_pin)
{

  memset(hx711, 0, sizeof(&hx711));

  //depending on the gain chosen here is the number of the SCLK pulses
  hx711->buffer_length = htim1.Init.RepetitionCounter + 1U;
  hx711->write_index = 0U;

  hx711->data_gpio = data_gpio;
  hx711->data_pin = data_pin;
  hx711->scale = 1;
  hx711->critical = 0U;
  hx711->new_patient = 1U;

  active_hx711 = hx711;
}

void start_measurement(void){
	if (HAL_TIM_OnePulse_Start(&htim2, TIM_CHANNEL_2) != HAL_OK)
	    {
	      Error_Handler();
	    }

	  if (HAL_TIM_Base_Start(&htim1) != HAL_OK)
	      {
	        Error_Handler();
	      }
}

void hx711_timer1_PWM_low_callback(hx711_t *hx711){

  uint8_t index = hx711->write_index;
  if (index < hx711->buffer_length)
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

void hx711_update_reading(hx711_t *hx711)
{
  long raw_value = transform_reading(hx711);
  hx711->raw_reading = raw_value;


  double adjusted = (double)raw_value - (double)hx711->offset;
  hx711->processed_reading = (float)((hx711->processed_reading + (adjusted / (double)hx711->scale))/hx711->measurement_count);
}


static long transform_reading(hx711_t *hx711){

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





void pack_data(hx711_t *hx711) {

    // to keep track of the current position in the array
    size_t offset = 0U;

    float processed_reading = hx711->processed_reading;
    memcpy(hx711->tx_buffer + offset, &processed_reading, sizeof(float));
    offset += sizeof(float);

    uint8_t critical = hx711->critical;

    memcpy(hx711->tx_buffer + offset, &critical, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    uint8_t new_patient = hx711->new_patient;
    memcpy(hx711->tx_buffer + offset, &new_patient, sizeof(uint8_t));
    offset += sizeof(uint8_t);


}
