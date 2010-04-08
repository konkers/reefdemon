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

#include "ow.h"
#include "ds18b20.h"


#define DS18B20_FAMILY_CODE		0x28

#define DS18B20_CMD_CONVERT_T		0x44
#define DS18B20_CMD_WRITE_SCRATCHPAD	0x4e
#define DS18B20_CMD_READ_SCRATCHPAD	0xbe
#define DS18B20_CMD_COPY_SCRATCHPAD	0x48
#define DS18B20_CMD_RECAL_E2		0xb8
#define DS18B20_CMD_READ_PS		0xb4

int16_t ds18b20_temps[DS18B20_MAX_SENSORS];

static void ds18b20_start(void)
{
	ow_reset();

	ow_skip_rom();

	ow_write_byte(DS18B20_CMD_CONVERT_T);
}


uint8_t ds18b20_ping(void)
{
	uint8_t i;
	uint8_t n = 0;

	if (!ow_read())
	    return 0;

	for (i = 0; i < ow_n_addrs; i++) {
		if (ow_addr(i)[0] == DS18B20_FAMILY_CODE) {
			ow_reset();
			ow_match_rom(i);
			ow_write_byte(DS18B20_CMD_READ_SCRATCHPAD);
			ds18b20_temps[n] = ow_read_byte();
			ds18b20_temps[n] |= ow_read_byte() << 8;

			ds18b20_temps[n] = ds18b20_temps[n];
		}

		n++;
		if (n == DS18B20_MAX_SENSORS)
			break;
	}

	ds18b20_start();
	return n;
}



void ds18b20_init(void)
{
	if (!ow_enumerate())
		return;

	ds18b20_start();
}



