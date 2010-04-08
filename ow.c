/*
 * Copyright 2010 Erik Gilling
 * Portions copyright 2007 James P Lynch
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
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "ow.h"
#include "util.h"
#include "pins.h"


#define OW_DQ		B_DQ
#define OW_DDR		DDRB
#define OW_PORT		PORTB
#define OW_PIN		PINB


#define OW_DELAY_A	US(6)
#define OW_DELAY_B	US(64)
#define OW_DELAY_C	US(60)
#define OW_DELAY_D	US(10)
#define OW_DELAY_E	US(9)
#define OW_DELAY_F	US(55)
#define OW_DELAY_G	US(0)
#define OW_DELAY_H	US(480)
#define OW_DELAY_I	US(70)
#define OW_DELAY_J	US(410)

struct ow_addr ow_addrs[OW_MAX_DEVICES];
uint8_t ow_n_addrs;

static inline void dq_low(void)
{
	OW_DDR |= _BV(OW_DQ);
}

static inline void dq_high(void)
{
	OW_DDR &= ~_BV(OW_DQ);
}


/*
 * Reset Timing
 *
 *   |     |                   |     |              |
 *  -+-----+                   | --  |    ----------+-
 *   |     |\                  |/  \ |   /          |
 *   |     | ------------------+    -+---           |
 *   |     |                   |     |              |
 *   |<-G->|<--------H-------->|<-I->|<------J----->|
 */


uint8_t ow_reset(void)
{
	uint8_t state;

	cli();
	delay(OW_DELAY_G);
	dq_low();
	delay(OW_DELAY_H);
	dq_high();
	delay(OW_DELAY_I);
	state = OW_PIN & _BV(OW_DQ);
	delay(OW_DELAY_J);
	sei();

	return !state;
}

/*
 * Write 1 timing
 *
 *      |     |                   |     |
 *  ----+     | ------------------+-----+---
 *      |\    |/                  |     |
 *      | ----+                   |     |
 *      |     |                   |     |
 *      |<-A->|<------------B---------->|
 */

void ow_write_1(void)
{
	cli();
	dq_low();
	delay(OW_DELAY_A);
	dq_high();
	delay(OW_DELAY_B);
	sei();
}

/*
 * Write 0 timing
 *
 *      |     |                   |     |
 *  ----+     |                   | ----+---
 *      |\    |                   |/    |
 *      | ------------------------+     |
 *      |     |                   |     |
 *      |<-----------C----------->|<-D->|
 */

void ow_write_0(void)
{
	cli();
	dq_low();
	delay(OW_DELAY_C);
	dq_high();
	delay(OW_DELAY_D);
	sei();
}

/*
 * Read timing
 *
 *      |     |        |          |     |
 *  ----+     | -------|----------+-----+---
 *      |\    |X       |          |/    |
 *      | ----+ -------+----------|     |
 *      |     |        |          |     |
 *      |<-A->|<--E--->|<------F------->|
 */

uint8_t ow_read(void)
{
	uint8_t state;

	cli();
	dq_low();
	delay(OW_DELAY_A);
	dq_high();
	delay(OW_DELAY_E);
	state = OW_PIN & _BV(OW_DQ);
	delay(OW_DELAY_F);
	sei();
	return state;
}

uint8_t ow_read_byte(void)
{
	uint8_t data = 0;
	uint8_t i;

	for (i = 0; i < 8; i++) {
		data >>= 1;
		if (ow_read())
			data |= 0x80;
	}

	return data;
}

void ow_write_byte(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (data & 0x1)
			ow_write_1();
		else
			ow_write_0();
		data >>= 1;
	}
}

static inline uint8_t get_bit(uint8_t *data, uint8_t bit)
{
	return data[bit >> 3] & (1 << (bit & 0x7));
}

static inline void set_bit(uint8_t *data, uint8_t bit, uint8_t val)
{
	if (val)
		data[bit >> 3] |= (1 << (bit & 0x7));
	else
		data[bit >> 3] &= ~(1 << (bit & 0x7));
}

struct ow_addr *ow_copy_addr(struct ow_addr *a)
{
	struct ow_addr *new_a;

	if (ow_n_addrs >= OW_MAX_DEVICES)
		return NULL;

	new_a = &ow_addrs[ow_n_addrs];
	ow_n_addrs++;
	memcpy(new_a, a, sizeof(*a));
	return new_a;
}


void ow_search_addr(uint8_t addr)
{
	uint8_t i;
	uint8_t b1, b2;
	struct ow_addr *a = &ow_addrs[addr];
	struct ow_addr *new_a;

	ow_reset();
	ow_write_byte(OW_CMD_SEARCH_ROM);

	for (i = 0; i < a->valid_bits; i++) {
		ow_read();
		ow_read();
		ow_write(get_bit(a->addr, i));
	}

	for (; i < 64; i++) {
		b1 = ow_read();
		b2 = ow_read();

		if (b1 && !b2) {
			set_bit(a->addr, i, 1);
		} else if (!b1 && b2) {
			set_bit(a->addr, i, 0);
		} else if (!b1 && !b2) {
			new_a = ow_copy_addr(a);
			set_bit(new_a->addr, i, 1);
			new_a->valid_bits = i + 1;

			set_bit(a->addr, i, 0);
		}
		ow_write(b1);
	}
}


uint8_t ow_enumerate(void)
{
	uint8_t addr = 0;

	memset(ow_addrs, 0x0, sizeof(ow_addrs));

	if (!ow_reset())
		return 0;

	ow_n_addrs = 1;

	while (addr < ow_n_addrs) {
		ow_search_addr(addr);
		addr++;
	}

	return ow_n_addrs;
}

void ow_match_rom(uint8_t addr)
{
	uint8_t *a = ow_addrs[addr].addr;
	uint8_t i;

	ow_write_byte(OW_CMD_MATCH_ROM);
	for (i = 0; i < 8; i++)
		ow_write_byte(a[i]);
}


void ow_init(void)
{
	OW_DDR &= ~_BV(OW_DQ);
	OW_PORT &= ~_BV(OW_DQ);
}
