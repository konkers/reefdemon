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


int main(void)
{
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
	lcd_print_string_P(10, 110, PSTR("konkers"), 
			   &font_pc8x8, 0x000, 0xfff);

	ow_init();
	ds18b20_init();

	while (1) {
		uint8_t i, n, w;

		n = ds18b20_ping();

		if (n) {
			for (i = 0; i < n; i++ ) {
				w = lcd_print_temp(10, 10 + 10 * i,
						   ds18b20_temps[i],
						   &font_pc8x8,
						   0xfff, 0x000);
				lcd_fill(10 + w, 10 + 10 * i,
					 50 - w, font_pc8x8.h, 0x000);
			}
		}

		w = lcd_print_hex(10, 50, time[2],
				  &font_pc8x8,0xfff, 0x000);
		w += lcd_print_char(10 + w, 50, ':',
				    &font_pc8x8,0xfff, 0x000);
		w += lcd_print_hex(10 + w, 50, time[1],
				   &font_pc8x8,0xfff, 0x000);
		w += lcd_print_char(10 + w, 50, ':',
				    &font_pc8x8,0xfff, 0x000);
		w += lcd_print_hex(10 + w, 50, time[0],
				   &font_pc8x8,0xfff, 0x000);

		lcd_draw_bitmap4(10, 60, &rd_logo);


		if (alert)
			PORTD |= _BV(D_ALERT);
		else
			PORTD &= ~_BV(D_ALERT);

		cli();
		if (ds18b20_temps[1] < (25 << 4))
			relays &= ~0x1;
		else if(ds18b20_temps[1] > ((25 << 4) + (1 << 3)))
			relays |= 0x1;
		sei();
	}

}

