/**
 * @file Clock.cpp
 * @brief Implementation of the clock utility functions for the game engine.
 * @details Handles time measurement for game loops, animations, and performance tracking.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Clock.h"

namespace gam300 {

    // Constructor - initialize previous_time to current time
    Clock::Clock() {
        // Get current time in microseconds
        auto now = std::chrono::high_resolution_clock::now();
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        auto epoch = now_us.time_since_epoch();
        m_previous_time = epoch.count(); // This will now store into int64_t
    }


    // Return time elapsed since previous call to delta(), reset clock time
    int64_t Clock::delta() { // Changed return type
        // Get current time in microseconds
        auto now = std::chrono::high_resolution_clock::now();
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        auto epoch = now_us.time_since_epoch();
        int64_t current_time = epoch.count(); // Changed to int64_t

        // Calculate elapsed time
        int64_t elapsed_time = current_time - m_previous_time; // Changed to int64_t

        // Handle potential error conditions (system clock adjusted, etc.)
        if (elapsed_time < 0) {
            elapsed_time = -1;  // Error indicator
        }

        // Reset previous time to current time
        m_previous_time = current_time;

        // Return elapsed time in microseconds
        return elapsed_time;
    }

    // Return time elapsed since previous call to delta(), don't reset clock time
    int64_t Clock::split() const { // Changed return type
        // Get current time in microseconds
        auto now = std::chrono::high_resolution_clock::now();
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        auto epoch = now_us.time_since_epoch();
        int64_t current_time = epoch.count(); // Changed to int64_t

        // Calculate elapsed time
        int64_t elapsed_time = current_time - m_previous_time; // Changed to int64_t

        // Handle potential error conditions (system clock adjusted, etc.)
        if (elapsed_time < 0) {
            elapsed_time = -1;  // Error indicator
        }

        // Return elapsed time in microseconds (without resetting the clock)
        return elapsed_time;
    }

} // end of namespace gam300