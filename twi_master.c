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

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pins.h"

enum twi_states {
	TWI_IDLE = 0,
	TWI_WRITE,
	TWI_READ,
	TWI_ERR,
};

static uint8_t twi_addr;
static uint8_t twi_data;
static volatile uint8_t twi_state;

static void twi_stop(void)
{
	uint8_t twcr = TWCR;

	twcr &= ~_BV(TWSTA);
	twcr |= _BV(TWINT) | _BV(TWSTO);
	TWCR = twcr;
}

static void twi_start(void)
{
	uint8_t twcr = TWCR;

	twcr &= ~_BV(TWSTO);
	twcr |= _BV(TWINT) | _BV(TWSTA);
	TWCR = twcr;
}

static void twi_continue(void)
{
	uint8_t twcr = TWCR;

	twcr &= ~_BV(TWSTA) & ~_BV(TWSTO);
	twcr |= _BV(TWINT);
	TWCR = twcr;
}

ISR( TWI_vect )
{
	uint8_t twsr = TWSR;

	switch (twsr) {
		/*
		 * A START condition has been transmitted
		 */
	case 0x08:
		TWDR = twi_addr;
		twi_continue();
		break;

		/*
		 * SLA+W has been transmitted ACK has been received
		 */
	case 0x18:
		if (twi_state == TWI_WRITE) {
			TWDR = twi_data;
			twi_continue();
		}
		break;

		/*
		 * SLA+W has been transmitted NOT ACK has been received
		 */
	case 0x20:
		twi_state = TWI_ERR;
		twi_stop();
		break;

		/*
		 * DATA has been transmitted ACK has been received
		 */
	case 0x28:
		twi_state = TWI_IDLE;
		twi_stop();
		break;

	default:
		twi_state = TWI_ERR;
		twi_stop();
		break;
	}
}

void twi_master_init(void)
{
	TWBR = 200;
	TWCR = _BV(TWEN) | _BV(TWIE);
}

void twi_send(uint8_t addr, uint8_t data)
{
	cli();
	if (twi_state == TWI_IDLE || twi_state == TWI_ERR) {
		twi_addr = addr << 1;
		twi_data = data;
		twi_state = TWI_WRITE;
		twi_start();
	}
	sei();
}
