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

#include "ds18b20.h"
#include "lcd.h"

#include "state.h"
#include "ui.h"

#include "util.h"

enum ui_states {
	UI_STATE_INIT = 0,
	UI_STATE_MAIN,
};

static uint8_t ui_state;

static uint8_t ui_print_time(uint8_t x, uint8_t y, struct font *font,
			  uint16_t fg_color, uint16_t bg_color)
{
	uint8_t w;

	w = lcd_print_hex(x, y, time[2], font, fg_color, bg_color);
	w += lcd_print_char(x + w, y, ':', font, fg_color, bg_color);
	w += lcd_print_hex(x + w, y, time[1], font, fg_color, bg_color);
	w += lcd_print_char(x + w, y, ':', font, fg_color, bg_color);
	w += lcd_print_hex(x + w, y, time[0], font, fg_color, bg_color);

	return w;
}

static void ui_value_box(uint8_t x, uint8_t y, uint8_t w,
			 PGM_P title, int32_t val, uint8_t digits,
			 struct font *font,
			 uint16_t val_color,
			 uint16_t fg_color, uint16_t bg_color)
{
	uint8_t w1 = (w - 2 - lcd_string_width_P(title, font)) / 2 + 1;
	lcd_draw_rect(x, y, w, 20, fg_color);
	lcd_fill(x + 1, y + 1, w1, 8, fg_color);
	w1 += lcd_print_string_P(x + w1, y + 1, title,
				 font, bg_color, fg_color);
	lcd_fill(x + w1, y + 1, w - w1 - 1, 8, fg_color);

	w1 = 2;
	w1 = lcd_print_fixed(x + w1, y + 10,
			     val, digits, font, val_color, bg_color);
}


static void ui_init()
{
	ui_state = UI_STATE_MAIN;
}

static void ui_main()
{
	uint8_t w;
	int32_t ph_display;

	/* top banner */
	lcd_fill(0, 0, 130, 1, 0xfff);
	lcd_fill(0, 1, 1, 8, 0xfff);
	lcd_fill(0, 9, 130, 1, 0xfff);
	w = ui_print_time(1, 1, &font_pc8x8, 0x000, 0xfff);
	lcd_fill( 1 + w, 1, 130 - w - 1, 8, 0xfff);

	lcd_draw_rect(4, 14, 122, 52, 0xfff);
	lcd_draw_graph(5, 15, 50,
		       temp_history, ARRAY_SIZE(temp_history),
		       temp_history_idx,
		       24 << 8, 26 << 8, set_point,
		       0xfff, 0xf00, 0x8f8, 0x00f);

	ui_value_box(4, 70, 40, PSTR("tank"), ds18b20_temps[1], 2, &font_pc8x8, 0x0f0, 0xfff, 0x000);
	ui_value_box(45, 70, 40, PSTR("hood"), ds18b20_temps[0], 2, &font_pc8x8, 0xf0f, 0xfff, 0x000);
	ui_value_box(86, 70, 40, PSTR("air"), ds18b20_temps[2], 2, &font_pc8x8, 0x0ff, 0xfff, 0x000);

	if (ph == 0) {
		ph = (uint32_t)adc_data[2] << PH_IIR_SHIFT;
	} else {
		ph -= ph >> PH_IIR_SHIFT;
		ph += adc_data[2];
	}

	ph_display = ph - (PH_7 << PH_IIR_SHIFT);
	ph_display <<= (8 - PH_IIR_SHIFT);
	ph_display *= (10 - 7);
	ph_display /= (PH_10 - PH_7);
	ph_display += (7 << 8);

	ui_value_box(4, 92, 40, PSTR("PH"), ph_display, 2, &font_pc8x8, 0xff0, 0xfff, 0x000);

}


void ui(void)
{
	switch(ui_state) {
	case UI_STATE_INIT:
		ui_init();
		break;

	case UI_STATE_MAIN:
		ui_main();
		break;
	}
}
