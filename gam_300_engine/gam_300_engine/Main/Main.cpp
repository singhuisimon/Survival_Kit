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

#include <GLFW/glfw3.h>
#include "Main.h"

int main(void) {
    // Initialize LogManager
    gam300::LogManager& logManager = gam300::LogManager::getInstance();
    if (logManager.startUp()) {
        // Failed to start LogManager
        printf("ERROR: Failed to start LogManager\n");
        return -1;
    }

    // Enable flush for immediate writing to disk
    logManager.setFlush(true);

    // Test the writeLog function
    logManager.writeLog("LogManager initialized successfully");
    logManager.writeLog("Testing formatting: %d, %s, %.2f", 42, "hello", 3.14159);

    // Initialize GLFW
    if (!glfwInit()) {
        logManager.writeLog("ERROR: Failed to initialize GLFW");
        logManager.shutDown();
        return -1;
    }

    logManager.writeLog("GLFW initialized successfully");

    // Create window
    GLFWwindow* window = glfwCreateWindow(640, 480, "LogManager Test", NULL, NULL);
    if (!window) {
        logManager.writeLog("ERROR: Failed to create GLFW window");
        glfwTerminate();
        logManager.shutDown();
        return -1;
    }

    logManager.writeLog("Window created with dimensions 640x480");
    glfwMakeContextCurrent(window);

    // Main loop
    logManager.writeLog("Entering main loop");
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // You can add periodic log messages here if needed
        // For example, log frame rate every few seconds

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    logManager.writeLog("Exiting main loop and cleaning up");
    glfwTerminate();

    // Properly shut down the LogManager
    logManager.shutDown();

    return 0;
}