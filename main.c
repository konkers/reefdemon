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

#include "pins.h"

static uint8_t alert;

uint8_t ticks;

#ifdef TIMER2_COMPA_vect
ISR( TIMER2_COMPA_vect )
#else
ISR( TIMER2_COMP_vect )
#endif
{
	ticks++;


	if ((ticks & 0x3f) == 0) {
		if (alert)
			PORTD |= _BV(D_ALERT);
		else
			PORTD &= ~_BV(D_ALERT);

		alert = !alert;
		twi_send(0x20, alert ? 0xa5 : 0x5a);
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

	sei();

	lcd_init();
	lcd_fill(0, 0, 131, 131, 0x000);
	lcd_print_string_P(10, 110, PSTR("konkers"), 
			   &font_pc8x8, 0x000, 0xfff);

	ow_init();

	while (1) {
		uint8_t i, j, w, n_addrs;
		uint8_t data[9];

		n_addrs = ow_enumerate();

		ow_reset();

		ow_skip_rom();

		ow_write_byte(0x44);

		while (!ow_read()) {
		}

		for (i = 0; i < n_addrs; i++ ) {
			ow_reset();

			ow_match_rom(i);

			ow_write_byte(0xBE);

			for (j = 0; j < 9; j++)
				data[j] = ow_read_byte();

			for (j = 0; j < 8; j++ ) {
				if ((j & 0x3) == 0)
					w = 0;

				w += lcd_print_hex(10 + w,
						   10 + 10 * (i * 3 + (j >> 2)),
						   ow_addr(i)[j],
						   &font_pc8x8,
						   0xfff, 0x000);
				w++;
			}

			lcd_print_temp(80, 10 + 10 * (i * 3),
				       data[0] | (data[1] << 8),
				       &font_pc8x8,
				       0xfff, 0x000);
		}

	}

}

