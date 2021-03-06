/* piezo-buzzer-bricklet
 * Copyright (C) 2011-2012 Olaf Lüke <olaf@tinkerforge.com>
 *
 * piezo-buzzer.c: Implementation of Piezo Buzzer Bricklet message
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "piezo-buzzer.h"

#include "brickletlib/bricklet_entry.h"
#include "bricklib/bricklet/bricklet_communication.h"
#include "bricklib/drivers/adc/adc.h"
#include "config.h"
#include "bricklib/utility/util_definitions.h"
#include "bricklib/utility/init.h"

#define MORSE_DIT_LENGTH 100
#define MORSE_DAH_LENGTH MORSE_DIT_LENGTH*3
#define MORSE_SPACE_LENGTH MORSE_DIT_LENGTH

void invocation(const ComType com, const uint8_t *data) {
	switch(((MessageHeader*)data)->fid) {
		case FID_BEEP: {
			beep(com, (Beep*)data);
			break;
		}

		case FID_MORSE_CODE: {
			morse_code(com, (MorseCode*)data);
			break;
		}

		default: {
			BA->com_return_error(data, sizeof(MessageHeader), MESSAGE_ERROR_CODE_NOT_SUPPORTED, com);
			break;
		}
	}
}

void constructor(void) {
	_Static_assert(sizeof(BrickContext) <= BRICKLET_CONTEXT_MAX_SIZE, "BrickContext too big");

    PIN_PIEZO.type = PIO_OUTPUT_0;
    PIN_PIEZO.attribute = PIO_DEFAULT;
    BA->PIO_Configure(&PIN_PIEZO, 1);

    BC->morse_pos = 60;
    BC->morse_duration = 0;
    BC->morse_buzz = false;
    BC->beep_duration = 0;
}

void destructor(void) {
    PIN_PIEZO.type = PIO_INPUT;
    PIN_PIEZO.attribute = PIO_PULLUP;
    BA->PIO_Configure(&PIN_PIEZO, 1);
}

void buzz(void) {
	PIN_PIEZO.pio->PIO_SODR = PIN_PIEZO.mask;
	SLEEP_NS(1000);
	PIN_PIEZO.pio->PIO_CODR = PIN_PIEZO.mask;
}

void beep(const ComType com, const Beep *data) {
	// Disable morse code beeping
	BC->morse_pos = MORSE_LENGTH;
    BC->morse_duration = 0;
    BC->morse_buzz = false;

	// Enable beep
	BC->beep_duration = data->duration;

	BA->com_return_setter(com, data);
}

void morse_code(const ComType com, const MorseCode *data) {
	// Disable beep
	BC->beep_duration = 0;

	// Enable morse code beeping
	BC->morse_pos = 0;
	BC->morse_duration = 0;
	BC->morse_buzz = false;

	for(uint8_t i = 0; i < MORSE_LENGTH; i++) {
		if(data->morse[i] != '.' && data->morse[i] != '-' && data->morse[i] != ' ') {
			BA->com_return_error(data, sizeof(MessageHeader), MESSAGE_ERROR_CODE_INVALID_PARAMETER, com);
			return;
		}

		BC->morse[i] = data->morse[i];
	}

	BA->com_return_setter(com, data);
}

void tick(const uint8_t tick_type) {
	if(tick_type & TICK_TASK_TYPE_MESSAGE) {
		if(BC->beep_finished) {
			BC->beep_finished = false;
			StandardMessage sm;
			BA->com_make_default_header(&sm, BS->uid, sizeof(StandardMessage), FID_BEEP_FINISHED);

			BA->send_blocking_with_timeout(&sm,
										   sizeof(StandardMessage),
										   *BA->com_current);
		}

		if(BC->morse_finished) {
			BC->morse_finished = false;
			StandardMessage sm;
			BA->com_make_default_header(&sm, BS->uid, sizeof(StandardMessage), FID_MORSE_CODE_FINISHED);

			BA->send_blocking_with_timeout(&sm,
										   sizeof(StandardMessage),
										   *BA->com_current);
		}
	}

	if(tick_type & TICK_TASK_TYPE_CALCULATION) {
		if(BC->beep_duration > 0) {
			BC->beep_duration--;
			buzz();

			if(BC->beep_duration == 0) {
				BC->beep_finished = true;
			}
		} else if(BC->morse_duration > 0) {
			BC->morse_duration--;
			if(BC->morse_buzz) {
				buzz();

				// Make pause after dit or dah finished
				if(BC->morse_duration == 0) {
					BC->morse_buzz = false;
					BC->morse_duration = MORSE_SPACE_LENGTH;
				}
			}
		}

		bool not_ready = true;

		while(BC->morse_pos < MORSE_LENGTH &&
			  BC->morse_duration == 0 &&
			  not_ready) {
			switch(BC->morse[BC->morse_pos]) {
				case '.':
					BC->morse_buzz = true;
					BC->morse_duration = MORSE_DIT_LENGTH;
					not_ready = false;
					break;
				case '-':
					BC->morse_buzz = true;
					BC->morse_duration = MORSE_DAH_LENGTH;
					not_ready = false;
					break;
				case ' ':
					BC->morse_buzz = false;
					BC->morse_duration = MORSE_SPACE_LENGTH;
					not_ready = false;
					break;
			}
			BC->morse_pos++;
			if(BC->morse_pos == MORSE_LENGTH) {
				BC->morse_finished = true;
			}
		}
	}
}
