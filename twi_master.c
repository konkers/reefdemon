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

#include "twi_master.h"

#define TWI_MAX_DEVICES		5

struct twi_device {
	void (* begin)(void);
	void (* end)(void);
};

enum twi_states {
	TWI_IDLE = 0,
	TWI_WRITE,
	TWI_READ,
	TWI_W_R,
	TWI_ERR,
};

static struct twi_device twi_devices[TWI_MAX_DEVICES];
static uint8_t twi_n_devices;
static uint8_t twi_device_idx;

static uint8_t twi_addr;
static volatile uint8_t twi_state;

uint8_t twi_data;

static inline uint8_t twi_busy(void)
{
	return (twi_state != TWI_IDLE) && (twi_state != TWI_ERR);
}

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
	twcr |= _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
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
		 * A repeated START condition has been transmitted
		 */
	case 0x10:
		if (twi_state == TWI_READ) {
			TWDR = twi_addr | 0x1;
			twi_continue();
		}
		break;

		/*
		 * SLA+W has been transmitted ACK has been received
		 */
	case 0x18:
		TWDR = twi_data;
		twi_continue();
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
		if (twi_state == TWI_W_R) {
			/* transmit a repeated start */
			twi_state = TWI_READ;
			twi_start();
		} else {
			twi_state = TWI_IDLE;
			twi_stop();
		}
		break;

		/*
		 * SLA+R hasb been transmitted ACK has been received
		 */
	case 0x40:
		twi_continue();
		break;

		/*
		 * data byte has been received; NOT ACK has been returned
		 */
	case 0x58:
		twi_data = TWDR;
		twi_state = TWI_IDLE;
		twi_stop();
		break;

	default:
		twi_state = TWI_ERR;
		twi_stop();
		break;
	}

	if (!twi_busy()) {
		twi_devices[twi_device_idx].end();
		twi_device_idx++;
		if (twi_device_idx == twi_n_devices)
			twi_device_idx = 0;
	}

}

void twi_master_init(void)
{
	TWBR = 200;
	TWCR = _BV(TWEN) | _BV(TWIE);
}

void twi_add_device(void (* begin)(void), void (* end)(void))
{
	if ((twi_n_devices + 1) < TWI_MAX_DEVICES) {
		twi_devices[twi_n_devices].begin = begin;
		twi_devices[twi_n_devices].end = end;
		twi_n_devices++;
	}
}

void twi_ping(void)
{
	cli();
	if (!twi_busy())
		twi_devices[twi_device_idx].begin();
	sei();
}


void twi_send(uint8_t addr, uint8_t data)
{
	twi_addr = addr << 1;
	twi_data = data;
	twi_state = TWI_WRITE;
	twi_start();
}

void twi_read_reg(uint8_t addr, uint8_t reg)
{
	twi_addr = addr << 1;
	twi_data = reg;
	twi_state = TWI_W_R;
	twi_start();
}
