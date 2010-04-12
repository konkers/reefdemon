/*
 * Copyright 2010 Erik Gilling
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <avr/interrupt.h>

#include "twi_master.h"
#include "lcd.h"
#include "ow.h"
#include "ds18b20.h"

#include "state.h"
#include "ui.h"
#include "util.h"

#include "pins.h"

volatile uint8_t alert;

uint8_t ticks;

#ifdef TIMER2_COMPA_vect
ISR( TIMER2_COMPA_vect )
#else
ISR( TIMER2_COMP_vect )
#endif
{
	ticks++;

	twi_ping();


	if ((ticks & 0x3f) == 0) {
		alert = !alert;
	}
}

static void set_timer2(uint8_t cs, uint8_t oc)
{
#ifdef TCCR2A
	TCCR2B = cs;
	/* set TC into CTC mode */
	TCCR2A = _BV(WGM21);
	OCR2A = oc;
	TIMSK2 |= _BV(OCIE2A);
#else
	TCCR2 = cs;
	OCR2 = oc;
	TIMSK |= _BV(OCIE2);
#endif
}

uint8_t new_time[3];
uint8_t time[3];
uint8_t time_idx;

void rtc_begin(void)
{
	twi_read_reg(0x68, time_idx);
}

void rtc_end(void)
{
	new_time[time_idx] = twi_data;

	time_idx++;
	if (time_idx > 2) {
		time[0] = new_time[0];
		time[1] = new_time[1];
		time[2] = new_time[2];
		time_idx = 0;
	}
}

uint8_t relays = 0xff;

void relay_begin(void)
{
	twi_send(0x20, relays);
}

void relay_end(void)
{
}

#define AVCC	(_BV(REFS0))

prog_uint8_t adc_channels[ADC_NUM_CHANNELS] = {
	AVCC | 0,
	AVCC | 1,
	AVCC | 6,
};

uint16_t adc_data[ADC_NUM_CHANNELS];

uint8_t adc_channel;

void adc_sample(void)
{
	ADMUX = pgm_read_byte(adc_channels + adc_channel);

	ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIE) |
		_BV(ADPS2) | _BV(ADPS1) |_BV(ADPS0);
}


ISR(ADC_vect)
{
	adc_data[adc_channel] = ADCW;

	adc_channel++;
	if (adc_channel == ADC_NUM_CHANNELS)
		adc_channel = 0;

	adc_sample();
}


uint32_t ph;
int32_t set_point;
int32_t temp_history[120];
uint8_t temp_history_idx;

int main(void)
{
	uint8_t last_min = 0;
	cli();

	DDRC = _BV(C_SCL);

	DDRD = _BV(D_LCD_BL) | _BV(D_LCD_SEL) | _BV(D_LCD_SCK) |
		_BV(D_LCD_MOSI) | _BV(D_LCD_RESET) | _BV(D_ALERT);

#if (FOSC == 18432000UL)
	/*
	 * Fosc = 18432000
	 * Fping = 100 Hz
	 *
	 * Fosc / Fping / 1024 = 180;
	 */
	set_timer2(_BV(CS22) | _BV(CS21) | _BV(CS20), 180);
#elif (FOSC == 7372800UL)
	/*
	 * Fosc = 7372800
	 * Fping = 100 Hz
	 *
	 * Fosc / Fping / 1024 = 72;
	 */
	set_timer2(_BV(CS22) | _BV(CS21) | _BV(CS20), 72);
#elif (FOSC == 16000000UL)
	/*
	 * Fosc = 16000000
	 * Fping = 100 Hz
	 *
	 * Fosc / Fping / 1024 = 156;
	 */
	set_timer2(_BV(CS22) | _BV(CS21) | _BV(CS20), 156);
#else
#error "unsupported FOSC freq"
#endif

	twi_master_init();
	twi_add_device(rtc_begin, rtc_end);
	twi_add_device(relay_begin, relay_end);
	sei();

	lcd_init();
	lcd_fill(0, 0, 131, 131, 0x000);

	ow_init();
	ds18b20_init();
	adc_sample();

	set_point = 25 << 8;

	while (1) {
		ds18b20_ping();

		if (last_min != time[1]) {
			temp_history[temp_history_idx] = ds18b20_temps[1];
			temp_history_idx++;
			if (temp_history_idx == ARRAY_SIZE(temp_history))
				temp_history_idx = 0;
			last_min = time[1];
		}

		if (ph == 0) {
			ph = (uint32_t)adc_data[2] << PH_IIR_SHIFT;
		} else {
			ph -= ph >> PH_IIR_SHIFT;
			ph += adc_data[2];
		}

		ui();

		if (alert)
			PORTD |= _BV(D_ALERT);
		else
			PORTD &= ~_BV(D_ALERT);

		cli();
		if (ds18b20_temps[1] < set_point)
			relays &= ~0x1;
		else if(ds18b20_temps[1] > (set_point + (1 << 4)))
			relays |= 0x1;
		sei();
	}

}

