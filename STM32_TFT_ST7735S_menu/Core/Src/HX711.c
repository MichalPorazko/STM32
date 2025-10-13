


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


void hx711_timer1_PWM_low_callback(void)
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
  }else
  {
    hx->buffer_ready = 1U;
    hx->write_index = 0U;
    active_hx711 = NULL;
  }

}


long transform_reading(hx711_t *hx711, uint8_t channel){

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


long read_average(hx711_t *hx711, int8_t times, uint8_t channel) {
	long sum = 0;
	for (int8_t i = 0; i < times; i++) {
		sum += transform_reading(hx711, channel);
	}
	return sum / times;
}


void set_offset(hx711_t *hx711, long offset, uint8_t channel){
	if(channel == CHANNEL_A) hx711->Aoffset = offset;
	else hx711->Boffset = offset;
}

double get_value(hx711_t *hx711, int8_t times, uint8_t channel) {
	long offset = 0;
	if(channel == CHANNEL_A) offset = hx711->Aoffset;
	else offset = hx711->Boffset;
	return read_average(hx711, times, channel) - offset;
}

//############################################################################################
void tare(hx711_t *hx711, uint8_t times, uint8_t channel) {
	transform_reading(hx711, channel); // Change channel
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
	transform_reading(hx711, channel);
	float scale = 0;
	if(channel == CHANNEL_A) scale = hx711->Ascale;
	else scale = hx711->Bscale;
	return get_value(hx711, times, channel) / scale;
}
