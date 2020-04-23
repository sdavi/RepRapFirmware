/*
 * Alligator Microstepping Settings
 *
 *  Created on: 15 May 2017
 *      Author: Marco Antonini
 */

#ifndef ALLIGATOR_MICROSTEPPING_H_
#define ALLIGATOR_MICROSTEPPING_H_

#include <inttypes.h>
#include "Core.h"


class Microstepping {

	public:

		static void Init() noexcept;
		static bool Set(uint8_t drive, uint8_t value) noexcept;
		static uint8_t Read( uint8_t drive ) noexcept;

	private:

		Microstepping() noexcept { };

};
#endif /* ALLIGATOR_MICROSTEPPING_H_ */
