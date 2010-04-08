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

#ifndef __ds18b20_h__
#define __ds18b20_h__

#define DS18B20_MAX_SENSORS	3

extern int16_t ds18b20_temps[DS18B20_MAX_SENSORS];

uint8_t ds18b20_ping(void);
void ds18b20_init(void);

#endif /* __ds18b20_h__ */
