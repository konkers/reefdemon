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

#ifndef __font_h__
#define __font_h__

#include <avr/pgmspace.h>

struct font_glyph
{
	uint8_t		w;
	int		idx;
};

struct font {
	prog_uint8_t		*data;
	struct font_glyph	*info;
	uint8_t			h;
	char			min_char;
	char			max_char;
};

extern struct font font_pc8x8;

#endif /* __font_h__ */
