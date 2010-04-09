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

#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "pngread.h"

void usage(void)
{
	printf( "png2font -f <pngfilename> -n <bitmapname>\n" );
}

pixel_t cmap[4];
int cmap_max = 4;
int cmap_len = 0;

int cmap_find(pixel_t color)
{
	int i = 0;

	while (i < cmap_len) {
		if (pixeleq(cmap[i], color))
			return i;
		i++;
	}

	if (i < cmap_max) {
		cmap[i] = color;
		cmap_len++;
		return i;
	}

	return -1;
}

unsigned color12(pixel_t c)
{
	return ((c.r >> 4) & 0xf) | (((c.g >> 4) & 0xf) << 4) | (((c.b >> 4) & 0xf) << 8);
}


int main( int argc, char *argv[] )
{
	unsigned long width, height;
	pixel_t *img_data;
	int c;
	char *name = NULL;
	char *fname = NULL;
	int option_index = 0;
	uint8_t data = 0;
	int bit = 0;
	int i;
	int byte = 0;

	while( 1 ) {
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"name", 1, 0, 'n'},
			{"file", 1, 0, 'f'},
			{0,0,0,0}
		};

		c = getopt_long( argc, argv, "hn:f:",
				 long_options, &option_index );

		if( c == -1 ) {
			break;
		}

		switch( c ) {
		case 'h':
			usage();
			return 0;

		case 'n':
			name=optarg;
			break;

		case 'f':
			fname=optarg;
			break; 

		default:
			fprintf( stderr, "unkown option -%c %d\n", (char)c, c );
			usage();
			return 1;
		}
	}

	if( fname == NULL ) {
		printf( "no file name given\n" );
		usage();
		return 1;
	}

	if( name == NULL ) {
		printf( "no bitmap name given\n" );
		usage();
		return 1;
	}

	if( pngread( fname, &width, &height,  &img_data ) < 0 ) {
		fprintf(stderr, "can't open %s", fname );
		return 1;
	}

	printf("#include \"bitmap.h\"\n\n");
	printf("static prog_uint8_t data[] = {\n");

	for (i = 0; i < width * height; i++) {
		int c = cmap_find(img_data[i]);
		if (c<0)
			c = 0;

		data |= (c & 0x3) << 6;

		bit += 2;
		if (bit == 8) {
			if (byte % 8 == 0)
				printf("\t");
			else
				printf(" ");

			printf("0x%02x,", data);

			if (byte % 8 == 7)
				printf("\n");

			byte++;

			bit = 0;
			data = 0;
		} else {
			data >>= 2;
		}

	}
	printf("\n};\n\n");

	printf("struct bitmap4 %s = {\n", name);
	printf("\t.data = data,\n");
	printf("\t.h = %lu,\n", height);
	printf("\t.w = %lu,\n", width);
	printf("\t.colors = {0x%03x, 0x%03x, 0x%03x, 0x%03x},\n",
	       color12(cmap[0]), color12(cmap[1]),
	       color12(cmap[2]), color12(cmap[3]));

	printf("\n};\n\n");

	return 0;
}


