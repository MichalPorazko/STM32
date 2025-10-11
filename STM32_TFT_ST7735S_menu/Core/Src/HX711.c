


#include "HX711.h"
#include "tim.h"


static hx711_t * volatile active_hx711 = NULL;


static void hx711_prepare_buffer(hx711_t *hx711)
{
  hx711->buffer_length = htim1.Init.RepetitionCounter;
  hx711->write_index = 0U;
  hx711->read_index = 0U;
  hx711->buffer_ready = 0U;
}


void hx711_init(hx711_t *hx711){

	hx711->Aoffset = 0;
	hx711->Ascale = 1.0f;
	hx711->Again = 0U;
	hx711->Boffset = 0;
	hx711->Bscale = 1.0f;
	hx711->Bgain = 0U;
	hx711_prepare_buffer(hx711);

}


void hx711_timer_edge_callback(void)
{
  hx711_t *hx = (hx711_t *)active_hx711;

  if (hx == NULL)
  {
    return;
  }

  uint8_t index = hx->write_index;
  if (index < hx->buffer_length)
  {
    hx->bit_buffer[index] = (uint8_t)(HAL_GPIO_ReadPin(hx->dat_gpio, hx->dat_pin) ? 1U : 0U);
    index++;
    hx->write_index = index;

    if (index >= hx->buffer_length)
    {
      hx->buffer_ready = 1U;
      hx->write_index = 0U;
      active_hx711 = NULL;
    }
  }
  else
  {
    hx->buffer_ready = 1U;
    hx->write_index = 0U;
    active_hx711 = NULL;
  }
}


void set_offset(hx711_t *hx711, long offset, uint8_t channel){
	if(channel == CHANNEL_A) hx711->Aoffset = offset;
	else hx711->Boffset = offset;
}


uint8_t shiftIn(hx711_t *hx711)
{
  uint8_t value = 0U;
  for (uint8_t i = 0U; i < 8U; ++i)
    {
      value <<= 1U;
      if (hx711->read_index < hx711->buffer_length)
          {
            value |= (hx711->bit_buffer[hx711->read_index] & 0x01U);
          }

          if (hx711->read_index < HX711_MAX_CAPTURE_BITS)
          {
            hx711->read_index++;
          }
        }

        return value;
}



long read(hx711_t *hx711, uint8_t channel){

	unsigned long value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	data[2] = shiftIn(hx711, 1);
	data[1] = shiftIn(hx711, 1);
	data[0] = shiftIn(hx711, 1);


	HAL_TIM_Base_Start_IT(&htim1);

	  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
	  {
		Error_Handler();
	  }

	// Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( (unsigned long)(filler) << 24
			| (unsigned long)(data[2]) << 16
			| (unsigned long)(data[1]) << 8
			| (unsigned long)(data[0]) );

	return (long)(value);
}


long read_average(hx711_t *hx711, int8_t times, uint8_t channel) {
	long sum = 0;
	for (int8_t i = 0; i < times; i++) {
		sum += read(hx711, channel);
	}
	return sum / times;
}

//############################################################################################
double get_value(hx711_t *hx711, int8_t times, uint8_t channel) {
	long offset = 0;
	if(channel == CHANNEL_A) offset = hx711->Aoffset;
	else offset = hx711->Boffset;
	return read_average(hx711, times, channel) - offset;
}

//############################################################################################
void tare(hx711_t *hx711, uint8_t times, uint8_t channel) {
	read(hx711, channel); // Change channel
	double sum = read_average(hx711, times, channel);
	set_offset(hx711, sum, channel);
}

//############################################################################################
void tare_all(hx711_t *hx711, uint8_t times) {
	tare(hx711, times, CHANNEL_A);
	tare(hx711, times, CHANNEL_B);
}

//############################################################################################
float get_weight(hx711_t *hx711, int8_t times, uint8_t channel) {
  // Read load cell
	read(hx711, channel);
	float scale = 0;
	if(channel == CHANNEL_A) scale = hx711->Ascale;
	else scale = hx711->Bscale;
	return get_value(hx711, times, channel) / scale;
}
