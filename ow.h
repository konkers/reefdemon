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

#ifndef __ow_h__
#define __ow_h__

#define OW_MAX_DEVICES		3

uint8_t ow_reset(void);
void ow_write_1(void);
void ow_write_0(void);
uint8_t ow_read(void);
uint8_t ow_read_byte(void);
void ow_write_byte(uint8_t data);
uint8_t ow_enumerate(void);
void ow_match_rom(uint8_t addr);
void ow_init(void);

#define OW_CMD_READ_ROM		0x33
#define OW_CMD_SKIP_ROM		0xcc
#define OW_CMD_MATCH_ROM	0x55
#define OW_CMD_SEARCH_ROM	0xf0
#define OW_CMD_OD_SKIP_ROM	0x3c
#define OW_CMD_OD_MATCH_ROM	0x69

struct ow_addr
{
	uint8_t		addr[8];
	uint8_t		valid_bits;
};

extern struct ow_addr ow_addrs[OW_MAX_DEVICES];
extern uint8_t ow_n_addrs;


static inline void ow_write(uint8_t val)
{
	if (val)
		ow_write_1();
	else
		ow_write_0();
}

static inline void ow_skip_rom(void)
{
	ow_write_byte(OW_CMD_SKIP_ROM);
}

static inline uint8_t *ow_addr(uint8_t addr)
{
	return ow_addrs[addr].addr;
}


#endif /* __ow_h__ */
