/**
 * @file GameManager.h
 * @brief Declaration of the Game Manager for the game engine.
 * @details Manages the game loop, frame timing, and overall game state.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "Manager.h"
#include <GLFW/glfw3.h>
#include <thread>  // Moved from GameManager.cpp
#include <chrono>  // Moved from GameManager.cpp

// Forward declaration for Clock (to avoid circular dependency)
namespace gam300 {
	class Clock;
}

// Two-letter acronym for easier access to manager.
#define GM gam300::GameManager::getInstance()

namespace gam300 {

	// Gam300 cofiguration file.
	const std::string CONFIG_FILENAME = "gam300-config.txt";

	// Default frame time (game loop time) in milliseconds (11.11 ms == 90 f/s).
	const int FRAME_TIME_DEFAULT = 11;

	class GameManager : public Manager {

	private:
		GameManager();                      // Private since a singleton.
		GameManager(GameManager const&);    // Don't allow copy.
		void operator=(GameManager const&); // Don't allow assignment.
		bool m_game_over;           // True -> game loop should stop.
		int m_step_count;           // Count of game loop iterations.

	public:
		// Get the singleton instance of the GameManager.
		static GameManager& getInstance();

		// Startup all GameManager services.
		int startUp();

		// Game manager only accepts step events.
		// Return false if other event.
		bool isValid(std::string event_name) const;

		// Shut down GameManager services.
		void shutDown();

		// Run game loop.
		void run();

		// Set game over status to indicated value.
		// If true (default), will stop game loop.
		void setGameOver(bool new_game_over = true);

		// Get game over status.
		bool getGameOver() const;

		// Return frame time.  
		// Frame time is target time for each game loop, in milliseconds.
		int getFrameTime() const;

		// Return game loop step count.
		int getStepCount() const;
	};

} // end of namespace df
#endif // __GAME_MANAGER_H__