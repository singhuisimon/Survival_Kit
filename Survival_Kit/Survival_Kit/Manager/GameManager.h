/**
 * @file GameManager.h
 * @brief Declaration of the Game Manager for the game engine.
 * @details Manages the game state, frame timing, and overall game systems.
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
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>

 // Forward declaration for Clock (to avoid circular dependency)
namespace gam300 {
    class Clock;
}

// Two-letter acronym for easier access to manager.
#define GM gam300::GameManager::getInstance()

namespace gam300 {

    // Default frame time (game loop time) in milliseconds (11.11 ms == 90 f/s).
    const int FRAME_TIME_DEFAULT = 11;

    class GameManager : public Manager {

    private:
        GameManager();                      // Private since a singleton.
        GameManager(GameManager const&);    // Don't allow copy.
        void operator=(GameManager const&); // Don't allow assignment.
        bool m_game_over;                   // True -> game loop should stop.
        int m_step_count;                   // Count of game loop iterations.

    public:
        /**
         * @brief Get the singleton instance of the GameManager.
         * @return Reference to the singleton instance.
         */
        static GameManager& getInstance();

        /**
         * @brief Startup all GameManager services.
         * @return 0 if successful, negative number if error.
         */
        int startUp() override;

        /**
         * @brief Check if an event is valid for the GameManager.
         * @param event_name The name of the event to check.
         * @return True if the event is valid, false otherwise.
         * @details Game manager only accepts step events.
         */
        bool isValid(std::string event_name) const;

        /**
         * @brief Shut down GameManager services.
         * @details Cleans up resources and shuts down the GameManager.
         */
        void shutDown() override;

        /**
         * @brief Update the game state for the current frame.
         * @param dt Delta time in seconds.
         * @details Processes input, updates systems, and handles game state.
         */
        void update(float dt);

        /**
         * @brief Set game over status to indicated value.
         * @param new_game_over The new game over status (default: true).
         * @details If true, will stop game loop.
         */
        void setGameOver(bool new_game_over = true);

        /**
         * @brief Get game over status.
         * @return True if game is over, false otherwise.
         */
        bool getGameOver() const;

        /**
         * @brief Return frame time.
         * @return Frame time in milliseconds.
         * @details Frame time is target time for each game loop.
         */
        int getFrameTime() const;

        /**
         * @brief Return game loop step count.
         * @return The current game loop step count.
         */
        int getStepCount() const;

        // Added: Scene management helper methods
        /**
         * @brief Work with entities loaded from serialization.
         * @param dt Delta time in seconds.
         * @details Demonstrates how to use entity lookup functionality.
         */
        void workWithSerializedEntities(float dt);

        /**
         * @brief Load a new scene, clearing existing entities first.
         * @param scenePath Path to the scene file to load.
         * @details Clears all existing entities before loading new scene.
         */
        void loadNewScene(const std::string& scenePath);

        /**
         * @brief Save current game state to a save slot.
         * @param saveSlot Name of the save slot (e.g., "slot1", "quicksave").
         * @details Saves current entities without clearing them from memory.
         */
        void saveCurrentGame(const std::string& saveSlot);
    };

} // end of namespace gam300
#endif // __GAME_MANAGER_H__