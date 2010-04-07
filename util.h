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

#ifndef __util_h__
#define __util_h__

#define US(x)		((unsigned)((x) * 2.2))

static inline void delay(unsigned ticks)
{
	unsigned i;

	for (i = 0; i < ticks; i++)
		__asm__ __volatile__("nop");
}

#endif /* __util_h__ */
