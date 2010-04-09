#include <avr/pgmspace.h>


struct bitmap4 {
	prog_uint8_t *data;
	uint8_t h;
	uint8_t w;
	uint16_t colors[4];
};

extern struct bitmap4 rd_logo;
