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

#ifndef __lcd_h__
#define __lcd_h__

#include "font.h"
#include "bitmap.h"

void lcd_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void lcd_draw_bitmap4(uint8_t x, uint8_t y, struct bitmap4 *bm);
uint8_t lcd_print_char(uint8_t x, uint8_t y, char c,
		       struct font *font, uint16_t fg_color,
		       uint16_t bg_color);
uint8_t lcd_print_string_P(uint8_t x, uint8_t y, PGM_P str,
			   struct font *font, uint16_t fg_color,
			   uint16_t bg_color);
uint8_t lcd_print_hex(uint8_t x, uint8_t y, uint8_t data,
		      struct font *font, uint16_t fg_color,
		      uint16_t bg_color);
uint8_t lcd_print_dec(uint8_t x, uint8_t y, uint8_t data,
		      struct font *font, uint16_t fg_color,
		      uint16_t bg_color);
uint8_t lcd_print_temp(uint8_t x, uint8_t y, uint16_t temp,
		   struct font *font, uint16_t fg_color,
		   uint16_t bg_color);
void lcd_init(void);

#endif /* __lcd_h__ */

