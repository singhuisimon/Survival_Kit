/**
 * @file GameManager.cpp
 * @brief Implementation of the Game Manager for the game engine.
 * @details Manages the game loop, frame timing, and overall game state.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

// Include managers and utility headers
#include "GameManager.h"
#include "LogManager.h"
#include "../Utility/Clock.h"

namespace gam300 {

    // Initialize singleton instance
    GameManager::GameManager() {
        setType("GameManager");
        m_game_over = false;
        m_step_count = 0;
    }

    // Get the singleton instance
    GameManager& GameManager::getInstance() {
        static GameManager instance;
        return instance;
    }

    // Start up the GameManager - initialize all required systems
    int GameManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Start the LogManager
        LogManager& logManager = LogManager::getInstance();
        if (logManager.startUp()) {
            // Failed to start LogManager
            return -1;
        }

        logManager.writeLog("GameManager::startUp() - LogManager started successfully");

        // Initialize step count
        m_step_count = 0;

        // Game is not over yet
        m_game_over = false;

        return 0;
    }

    // Check if an event is valid for the GameManager
    bool GameManager::isValid(std::string event_name) const {
        // GameManager only accepts "step" events
        return (event_name == "step");
    }

    // Shut down the GameManager - clean up all resources
    void GameManager::shutDown() {
        // Log shutdown
        LogManager& logManager = LogManager::getInstance();
        logManager.writeLog("GameManager::shutDown() - Shutting down GameManager");

        // Set game over
        setGameOver();

        // Shut down LogManager
        logManager.shutDown();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Run the main game loop
    void GameManager::run() {
        // Log start of game loop
        LogManager& logManager = LogManager::getInstance();
        logManager.writeLog("GameManager::run() - Starting game loop");

        // Create a clock for timing
        Clock clock;

        // Variables for timing
        long int elapsed_time = 0;
        long int sleep_time = 0;

        // Main game loop
        while (!m_game_over) {
            
            glfwPollEvents();

            // Increment step count
            m_step_count++;

            // Log every 100 steps
            if (m_step_count % 100 == 0) {
                logManager.writeLog("GameManager::run() - Step count: %d", m_step_count);
            }

            // Start of loop timing
            clock.delta();

            // This is where game logic would be processed
            // For now, just a dummy statement
            if (m_step_count % 1000 == 0) {  // Reduced logging frequency to avoid filling log file
                logManager.writeLog("GameManager::run() - Step %d", m_step_count);
            }

            // End of loop timing
            elapsed_time = clock.split();

            // Convert frame time from milliseconds to microseconds
            long int frame_time_us = getFrameTime() * 1000;

            // Calculate sleep time (need to ensure it's not negative)
            sleep_time = frame_time_us - elapsed_time;

            // Only sleep if positive sleep time
            if (sleep_time > 0) {
                // Convert microseconds to milliseconds for sleep
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
            }
            else {
                // If we're behind, log that we're not keeping up
                logManager.writeLog("GameManager::run() - Frame running behind: %ld us", -sleep_time);
            }
        }

        // Log end of game loop
        logManager.writeLog("GameManager::run() - Game loop ended with %d steps", m_step_count);
    }

    // Set game over status
    void GameManager::setGameOver(bool new_game_over) {
        m_game_over = new_game_over;

        // Log game over state change if setting to true
        if (new_game_over) {
            LogManager& logManager = LogManager::getInstance();
            logManager.writeLog("GameManager::setGameOver() - Game over set to true");
        }
    }

    // Get game over status
    bool GameManager::getGameOver() const {
        return m_game_over;
    }

    // Get frame time in milliseconds
    int GameManager::getFrameTime() const {
        // For now, return the default value
        // In a more complete implementation, this would be read from a config file
        return FRAME_TIME_DEFAULT;
    }

    // Get step count
    int GameManager::getStepCount() const {
        return m_step_count;
    }

} // end of namespace gam300