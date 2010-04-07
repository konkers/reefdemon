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
#include <avr/io.h>

#include "lcd.h"
#include "pins.h"

#undef EPSON
#define PHILLIPS


/* Epson commands */
#define DISON		0xAF
#define DISOFF		0xAE
#define DISNOR		0xA6
#define DISINV		0xA7
#define SLPIN		0x95
#define SLPOUT		0x94
#define COMSCN		0xBB
#define DISCTL		0xCA
#define PASET		0x75
#define CASET		0x15
#define DATCTL		0xBC
#define RGBSET8		0xCE
#define RAMWR		0x5C
#define RAMRD		0x5D
#define PTLIN		0xA8
#define PTLOUT		0xA9
#define RMWIN		0xE0
#define RMWOUT		0xEE
#define ASCSET		0xAA
#define SCSTART		0xAB
#define OSCON		0xD1
#define OSCOFF		0xD2
#define PWRCTR		0x20
#define VOLCTR		0x81
#define VOLUP		0xD6
#define VOLDOWN		0xD7
#define TMPGRD		0x82
#define EPCTIN		0xCD
#define EPCOUT		0xCC
#define EPMWR		0xFC
#define EPMRD		0xFD
#define EPSRRD1		0x7C
#define EPSRRD2		0x7D
#define NOP		0x25

/* Phillips commands */
#define NOPP		0x00
#define BSTRON		0x03
#define SLEEPIN		0x10
#define SLEEPOUT	0x11
#define NORON		0x13
#define INVOFF		0x20
#define INVON		0x21
#define SETCON		0x25
#define DISPOFF		0x28
#define DISPON		0x29
#define CASETP		0x2A
#define PASETP		0x2B
#define RAMWRP		0x2C
#define RGBSET		0x2D
#define MADCTL		0x36
#define COLMOD		0x3A
#define DISCTR		0xB9
#define EC		0xC0

static inline void bit(uint8_t d, uint8_t data, uint8_t mask)
{
	if (data & mask) {
		PORTD = d | _BV(D_LCD_MOSI);
		PORTD = d | _BV(D_LCD_SCK) | _BV(D_LCD_MOSI);
	} else {
		PORTD = d;
		PORTD = d | _BV(D_LCD_SCK);
	}
}

void lcd_data(uint8_t data)
{
	uint8_t d = PORTD;

	d &= ~_BV(D_LCD_SEL) & ~_BV(D_LCD_SCK) & ~_BV(D_LCD_MOSI);

	/* D/C_N = 1 (data) */
	PORTD = d | _BV(D_LCD_MOSI);
	PORTD = d | _BV(D_LCD_MOSI) | _BV(D_LCD_SCK);

	bit(d, data, 0x80);
	bit(d, data, 0x40);
	bit(d, data, 0x20);
	bit(d, data, 0x10);
	bit(d, data, 0x08);
	bit(d, data, 0x04);
	bit(d, data, 0x02);
	bit(d, data, 0x01);

	PORTD = d;
	PORTD = d | _BV(D_LCD_SEL);
}

void lcd_cmd(uint8_t cmd)
{
	uint8_t d = PORTD;

	d &= ~_BV(D_LCD_SEL) & ~_BV(D_LCD_SCK) & ~_BV(D_LCD_MOSI);

	/* D/C_N = 0 (data) */
	PORTD = d;
	PORTD = d | _BV(D_LCD_SCK);

	bit(d, cmd, 0x80);
	bit(d, cmd, 0x40);
	bit(d, cmd, 0x20);
	bit(d, cmd, 0x10);
	bit(d, cmd, 0x08);
	bit(d, cmd, 0x04);
	bit(d, cmd, 0x02);
	bit(d, cmd, 0x01);

	PORTD = d;
	PORTD = d | _BV(D_LCD_SEL);
}

#ifdef EPSON
static inline void epson_cmd(uint8_t cmd)
{
	lcd_cmd(cmd);
}
static inline void epson_data(uint8_t data)
{
	lcd_data(data);
}
#else
static inline void epson_cmd(uint8_t cmd)
{
}
static inline void epson_data(uint8_t data)
{
}
#endif

#ifdef PHILLIPS
static inline void phillips_cmd(uint8_t cmd)
{
	lcd_cmd(cmd);
}
static inline void phillips_data(uint8_t data)
{
	lcd_data(data);
}
#else
static inline void phillips_cmd(uint8_t cmd)
{
}
static inline void phillips_data(uint8_t data)
{
}
#endif

void lcd_set_aperature(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	epson_cmd(CASET);
	phillips_cmd(CASETP);
	lcd_data(x);
	lcd_data(x + w - 1);

	epson_cmd(PASET);
	phillips_cmd(PASETP);
	lcd_data(y);
	lcd_data(y + h - 1);
}

void lcd_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
	int i;
	color = ~color;

	lcd_set_aperature(x, y, w, h);

	epson_cmd(RAMWR);
	phillips_cmd(RAMWRP);

	for (i = 0; i < (w * h + 1) / 2; i++) {
		lcd_data((color >> 4) & 0xFF);
		lcd_data(((color & 0xF) << 4) | ((color >> 8) & 0xf));
		lcd_data(color & 0xFF);
	}
}

int lcd_print_char(uint8_t x, uint8_t y, char c,
		   struct font *font, uint16_t fg_color,
		   uint16_t bg_color)
{
	int i, j;
	struct font_glyph glyph;
	uint8_t data;

	bg_color = ~bg_color;
	fg_color = ~fg_color;

	if (c < font->min_char || c > font->max_char)
		return 0;

	memcpy_P(&glyph, font->info + c - font->min_char, sizeof(glyph));

	lcd_set_aperature(x, y, glyph.w, font->h);

	if (glyph.w == 0)
		return 0;

	epson_cmd(RAMWR);
	phillips_cmd(RAMWRP);

	for (i = 0; i < (glyph.w * font->h) / 8; i++) {
		data = pgm_read_byte(font->data + glyph.idx + i);
		for (j = 0; j < 4; j++) {
			uint16_t c1 = data & 0x1 ? fg_color : bg_color;
			uint16_t c2 = data & 0x2 ? fg_color : bg_color;

			lcd_data((c1 >> 4) & 0xFF);
			lcd_data(((c1 & 0xF) << 4) | ((c2 >> 8) & 0xF));
			lcd_data(c2 & 0xFF);

			data >>= 2;
		}

	}

	return glyph.w;
}

int lcd_print_string_P(uint8_t x, uint8_t y, PGM_P str,
		       struct font *font, uint16_t fg_color,
		       uint16_t bg_color)
{
	char c;
	int w = 0;
	while ((c = pgm_read_byte(str++))) {
		w += lcd_print_char(x + w, y, c, font, fg_color, bg_color);
		lcd_fill(x + w, y, 1, font->h, bg_color);
		w++;
	}
	return w;
}


void lcd_init(void)
{
	uint8_t d = PORTD;

	d &= ~_BV(D_LCD_SCK) & ~_BV(D_LCD_MOSI);
	d |= _BV(D_LCD_RESET) | _BV(D_LCD_SEL) | _BV(D_LCD_BL);

	PORTD = d;

	/* display control */
	epson_cmd(DISCTL);
	epson_data(0x0c);
	epson_data(0x20);
	epson_data(0x00);
	epson_data(0x01);

	/* common scanning direction */
	epson_cmd(COMSCN);
	epson_data(0x01);

	/* internal oscillator on */
	epson_cmd(OSCON);

	/* wake up */
	epson_cmd(SLPOUT);
	phillips_cmd(SLEEPOUT);

	/* turn everything on */
	epson_cmd(PWRCTR);
	epson_data(0x0f);

	/* booster on */
	phillips_cmd(BSTRON);

	/* invert display mode */
	epson_cmd(DISINV);
	phillips_cmd(INVON);

	/* data control */
	epson_cmd(DATCTL);
	epson_data(0x03);
	epson_data(0x00);
	epson_data(0x02);

	/* memory access control */
	phillips_cmd(MADCTL);
	phillips_data(0xc8);

	/* color mode */
	phillips_cmd(COLMOD);
	phillips_data(0x03);

	/* contrast */
	epson_cmd(VOLCTR);
	epson_data(0x24);
	epson_data(0x03);

	phillips_cmd(SETCON);
	phillips_data(0x30);

	epson_cmd(NOP);
	epson_cmd(NOPP);

	/* display on */
	epson_cmd(DISON);
	phillips_cmd(DISPON);
}

