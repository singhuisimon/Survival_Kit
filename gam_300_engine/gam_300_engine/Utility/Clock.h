/**
 * @file Clock.h
 * @brief Declaration of the Clock utility class for the game engine.
 * @details Handles time measurement for game loops, animations, and performance tracking.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __CLOCK_H__
#define __CLOCK_H__

 // Include chrono for high_resolution_clock (moved from Clock.cpp)
#include <chrono>
#include <ctime>

namespace gam300 {

	class Clock {

	private:
		long int m_previous_time; // Previous call to delta() (in microseconds).

	public:
		// Sets previous_time to current time.
		Clock();

		/**
		 * @brief Return time elapsed since delta() was called.
		 * @return Elapsed time in microseconds, -1 if error.
		 * @details Resets clock time.
		 */
		long int delta();

		/**
		 * @brief Return time elapsed since delta() was called.
		 * @return Elapsed time in microseconds, -1 if error.
		 * @details Does not reset clock time.
		 */
		long int split() const;
	};

} // end of namespace gam300
#endif // __CLOCK_H__