
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
