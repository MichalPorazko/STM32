


#include "HX711.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "stdbool.h"
#include "menu.h"
#include "rtc.h"


hx711_t *active_hx711 = NULL;
uint8_t bla  = 0;



void reset_parameters( hx711_t *hx711){

	hx711->sclk_pulses = 0U;
	hx711->critical = 0U;
	hx711->write_index = 0U;
	hx711->measurement_count = 0U;
	hx711->raw_reading = 0;
	hx711->start_measurement = 0U;

	hx711->sum = 0.0f;
	hx711->processed_reading = 0.0f;

}


void hx711_init( hx711_t *hx711, GPIO_TypeDef *data_gpio, uint16_t data_pin, GPIO_TypeDef *clk_gpio, uint16_t clk_pin)
{


  reset_parameters(hx711);
  hx711->data_gpio = data_gpio;
  hx711->data_pin = data_pin;

  hx711->clk_gpio = clk_gpio;
  hx711->clk_pin = clk_pin;

  hx711->scale = -1.017;
  hx711->new_patient = 1U;
  hx711->offset = -39878.57;
  hx711->gain = 3;

  active_hx711 = hx711;
}


static uint8_t shiftIn(hx711_t *hx711) {
    uint8_t value = 0;
    uint8_t i;

    for(i = 0; i < 8; ++i) {
    	HAL_GPIO_WritePin(HX_SCK_GPIO_Port, HX_SCK_Pin, GPIO_PIN_SET);
        value |= HAL_GPIO_ReadPin(HX_DT_GPIO_Port, HX_DT_Pin) << (7 - i);
        HAL_GPIO_WritePin(HX_SCK_GPIO_Port, HX_SCK_Pin, GPIO_PIN_RESET);
    }
    return value;
}

static bool is_ready(hx711_t *hx711) {
	if(HAL_GPIO_ReadPin(HX_DT_GPIO_Port, HX_DT_Pin) == GPIO_PIN_RESET){
		return 1;
	}
	return 0;
}


static void wait_ready(hx711_t *hx711) {
	// Wait for the chip to become ready.
	while (!is_ready(hx711)) {
	}
}

static long read(hx711_t *hx711){

	wait_ready(hx711);

	unsigned long value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	//noInterrupts();

	data[2] = shiftIn(hx711);
	data[1] = shiftIn(hx711);
	data[0] = shiftIn(hx711);

	for (unsigned int i = 0; i < hx711->gain; i++) {
		HAL_GPIO_WritePin(HX_SCK_GPIO_Port, HX_SCK_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(HX_SCK_GPIO_Port, HX_SCK_Pin, GPIO_PIN_RESET);
	}

	//interrupts();

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

static long read_average(hx711_t *hx711, int8_t times) {

	long  sum = 0.0f;

	for (int8_t i = 0; i < times; i++) {
		sum += read(hx711);
	}
	return sum / times;
}


static double get_value(hx711_t *hx711, int8_t times) {

	long offset = hx711->offset;
	long avg = read_average(hx711, times);
	return avg - offset;

}


void tare(hx711_t *hx711, uint8_t times) {
	float sum = read_average(hx711, times);
	hx711->offset = sum;
}


void tare_all(hx711_t *hx711, uint8_t times) {
	tare(hx711, times);
}

static float get_weight(hx711_t *hx711, int8_t times) {
	return get_value(hx711, times) / hx711->scale;
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

void start_measurement(hx711_t *hx711){

	//hx711->processed_reading = get_weight(active_hx711, 10);
	bla++;
	hx711->processed_reading = bla;
	menu_refresh();
	pack_data(hx711);
	if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *)( active_hx711->tx_buffer), HX711_TX_BUFFER_SIZE) == HAL_OK){
						active_hx711->measurement_count = 0U;
						active_hx711->tx_in_progress = 1U;
	}

	sleep();


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



