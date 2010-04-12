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

#ifndef __state_h__
#define __state_h__

#define PH_7		0x22d
#define PH_10		0x367
#define PH_IIR_SHIFT	4

extern uint32_t ph;

extern int32_t temp_history[120];
extern uint8_t temp_history_idx;

extern uint8_t time[3];

extern int32_t set_point;

#define ADC_NUM_CHANNELS 3

extern uint16_t adc_data[ADC_NUM_CHANNELS];

#endif /* __state_h__ */
