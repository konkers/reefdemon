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
	lcd_fill(10, 10, 50, 50, 0x00f);
	lcd_fill(60, 10, 50, 50, 0x0f0);
	lcd_fill(10, 60, 50, 50, 0xf00);
	lcd_fill(60, 60, 50, 50, 0xfff);
	lcd_print_string_P(10, 110, PSTR("konkers"), 
			   &font_pc8x8, 0x000, 0xfff);
	while (1) {
	}

}

