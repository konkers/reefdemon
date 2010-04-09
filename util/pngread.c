/*
 * Copyright 2005-2010 Erik Gilling
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
#include <stdlib.h>

#include <png.h>

#include "pngread.h"

int pngread(char *file, unsigned long *width, 
	    unsigned long *height, pixel_t **image_data_ptr)
{
	FILE         *infile;         /* PNG file pointer */
	png_structp   png_ptr;        /* internally used by libpng */
	png_infop     info_ptr;       /* user requested transforms */

	uint8_t         *image_data;      /* raw png image data */
	char         sig[8];           /* PNG signature array */

	int           bit_depth;
	int           color_type;
	int  interlace_type;
	unsigned int rowbytes;         /* raw bytes at row n in image */

	image_data = NULL;
	int i;
	png_bytepp row_pointers = NULL;

	/* Open the file. */
	infile = fopen(file, "rb");
	if (!infile) {
		return -1;
	}


	/*
	 * 		13.3 readpng_init()
	 */

	/* Check for the 8-byte signature */
	fread(sig, 1, 8, infile);

	if (!png_check_sig((unsigned char *) sig, 8)) {
		fclose(infile);
		return -1;
	}

	/*
	 * Set up the PNG structs
	 */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose(infile);
		return -1;    /* out of memory */
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		fclose(infile);
		return -1;    /* out of memory */
	}

	/*
	 * block to handle libpng errors,
	 * then check whether the PNG file had a bKGD chunk
	 */
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(infile);
		return -1;
	}

	/*
	 * takes our file stream pointer (infile) and
	 * stores it in the png_ptr struct for later use.
	 */
	/* png_ptr->io_ptr = (png_voidp)infile;*/
	png_init_io(png_ptr, infile);

	/*
	 * lets libpng know that we already checked the 8
	 * signature bytes, so it should not expect to find
	 * them at the current file pointer location
	 */
	png_set_sig_bytes(png_ptr, 8);

	/* Read the image info.*/

	/*
	 * reads and processes not only the PNG file's IHDR chunk 
	 * but also any other chunks up to the first IDAT 
	 * (i.e., everything before the image data).
	 */

	/* read all the info up to the image data  */
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, width, height, &bit_depth, 
		     &color_type, &interlace_type, NULL, NULL);

	/* Set up some transforms. */
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(png_ptr);
	}
	if (bit_depth > 8) {
		png_set_strip_16(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	if( interlace_type != PNG_INTERLACE_NONE ) {
		printf( "INTERLACED!\n" );
	}

	/* Update the png info struct.*/
	png_read_update_info(png_ptr, info_ptr);

	/* Rowsize in bytes. */
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	/* Allocate the image_data buffer. */
	if ((image_data = (uint8_t *) malloc(rowbytes * *height))==NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return -1;
	}

	if ((row_pointers = (png_bytepp)malloc(*height*sizeof(png_bytep))) == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		free(image_data);
		image_data = NULL;
		return -1;
	}

	/* set the individual row_pointers to point at the correct offsets */

	for (i = 0;  i < *height;  ++i)
		row_pointers[i] = ((uint8_t*)image_data) + i*rowbytes;

	/* now we can go ahead and just read the whole image */
	png_read_image(png_ptr, row_pointers);

	/* and we're done!  (png_read_end() can be omitted if no processing of
	 * post-IDAT text/time/etc. is desired) */

	/* Clean up. */
	free(row_pointers);

	/* Clean up. */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(infile);

	*image_data_ptr = (pixel_t *) image_data;

	return 0;
}

void pngfree(pixel_t *img_data)
{
	free( img_data );
}
