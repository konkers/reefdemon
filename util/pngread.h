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

#include <stdint.h>

#ifndef __pngread_h__
#define __pngread_h__

typedef struct {
	uint8_t r,g,b;
} __attribute__ ((packed)) pixel_t;

#define CMPVAL 10

static inline int pixeleq( pixel_t a, pixel_t b ) {
	return (a.r - b.r)<CMPVAL && (a.r - b.r)>-CMPVAL && 
		(a.g - b.g)<CMPVAL && (a.g - b.g)>-CMPVAL && 
		(a.b - b.b)<CMPVAL && (a.b - b.b)>-CMPVAL;
}

static inline int pixellux( pixel_t a ) {
	return (a.r + a.g + a.b)/3;
}

int pngread(char *file, unsigned long *width, 
	    unsigned long *height, pixel_t **image_data_ptr);

void pngfree(pixel_t *img_data);

#endif /* __pngread_h__ */
