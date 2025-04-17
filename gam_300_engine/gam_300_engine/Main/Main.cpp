/**
 * @file main.cpp
 * @brief Entry point of the game engine application.
 * @details Initializes the Game_Manager, loads configurations, sets up the window, and runs the main loop.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Main.h"

int main(void) {
    // Initialize GameManager
    if (GM.startUp()) {
        // Failed to start GameManager (which should already have started LogManager)
        printf("ERROR: Failed to start GameManager\n");
        return -1;
    }

    // Get reference to LogManager (already started by GameManager)
    LM.writeLog("Main: GameManager initialized successfully");

    // Initialize GLFW
    if (!glfwInit()) {
        LM.writeLog("ERROR: Failed to initialize GLFW");
        GM.shutDown();
        return -1;
    }

    LM.writeLog("GLFW initialized successfully");

    // Create window
    GLFWwindow* window = glfwCreateWindow(640, 480, "Game Engine Test", NULL, NULL);
    if (!window) {
        LM.writeLog("ERROR: Failed to create GLFW window");
        glfwTerminate();
        GM.shutDown();
        return -1;
    }

    LM.writeLog("Window created with dimensions 640x480");
    glfwMakeContextCurrent(window);

    // Set up a key callback to end the game
    glfwSetKeyCallback(window, [](GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            GM.setGameOver(true);
        }
        });

    // Create a clock for timing
    gam300::Clock clock;

    // Variables for timing
    long int elapsed_time = 0;
    long int sleep_time = 0;

    // Main game loop
    LM.writeLog("Starting main game loop");
    while (!GM.getGameOver() && !glfwWindowShouldClose(window)) {
        // Process events
        glfwPollEvents();

        // Increment step count in GameManager
        // Note: You might need to add a method in GameManager to increment step count

        // Start of loop timing
        clock.delta();

        // Render frame 
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers
        glfwSwapBuffers(window);

        // End of loop timing
        elapsed_time = clock.split();

        // Convert frame time from milliseconds to microseconds
        long int frame_time_us = GM.getFrameTime() * 1000;

        // Calculate sleep time (need to ensure it's not negative)
        sleep_time = frame_time_us - elapsed_time;

        // Only sleep if positive sleep time
        if (sleep_time > 0) {
            // Convert microseconds to milliseconds for sleep
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
        }
        else {
            // If we're behind, log that we're not keeping up
            LM.writeLog("Main: Frame running behind: %ld us", -sleep_time);
        }
    }

    // Cleanup
    LM.writeLog("Cleaning up GLFW resources");
    glfwTerminate();

    // Properly shut down the GameManager (which will also shut down LogManager)
    GM.shutDown();

    return 0;
}