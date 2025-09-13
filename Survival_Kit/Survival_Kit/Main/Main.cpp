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
    //// Initialize GameManager
    //if (GM.startUp()) {
    //    // Failed to start GameManager
    //    printf("ERROR: Failed to start GameManager\n");
    //    return -1;
    //}

    // Get reference to LogManager (already started by GameManager)
    LM.writeLog("Main: GameManager initialized successfully");

    // Initialize GLFW
    if (!glfwInit()) {
        LM.writeLog("ERROR: Failed to initialize GLFW");
        GM.shutDown();
        return -1;
    }

    LM.writeLog("GLFW initialized successfully");

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Additional settings
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_RED_BITS, 8); glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8); glfwWindowHint(GLFW_ALPHA_BITS, 8);

    // Create window
    GLFWwindow* window = glfwCreateWindow(640, 480, "Survival_Kit", NULL, NULL);
    if (!window) {
        LM.writeLog("ERROR: Failed to create GLFW window");
        glfwTerminate();
        GM.shutDown();
        return -1;
    }

    LM.writeLog("Window created with dimensions 640x480");
    glfwMakeContextCurrent(window);

    // Initialize graphics manager
    // KENNY TESTING: Remove this start up as GM startup GFXM in the next block
    //GFXM.startUp(); 

    //// Load OpenGL function pointers with GLAD
    //if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    //    LM.writeLog("Failed to initialize OpenGL function pointers!");
    //    std::cerr << "Failed to initialize OpenGL function pointers!" << std::endl;
    //    glfwDestroyWindow(window);
    //    glfwTerminate();
    //    return -1;
    //}
    //else {
    //    LM.writeLog("GLAD initialized successfully.");
    //    std::cout << "GLAD initialized successfully." << std::endl;
    //}

    // Initialize GameManager
    if (GM.startUp()) {
        // Failed to start GameManager
        printf("ERROR: Failed to start GameManager\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Register window with InputManager
    IM.setWindow(window);
    LM.writeLog("InputManager initialized successfully");

    // Create a clock for timing
    gam300::Clock clock;

    // Variables for timing
    int64_t elapsed_time = 0;
    int64_t sleep_time = 0;

    // Main game loop
    LM.writeLog("Starting main game loop");
    while (!GM.getGameOver() && !glfwWindowShouldClose(window)) {
        // Process events
        glfwPollEvents();

        // Update input system
        IM.update();

        // Start of loop timing
        clock.delta();

        // Update all systems (including InputSystem)
        EM.updateSystems(GM.getFrameTime() / 1000.0f);

        // Render frame 
        glClear(GL_COLOR_BUFFER_BIT);

        GFXM.update();

        // Swap buffers
        glfwSwapBuffers(window);

        // End of loop timing
        elapsed_time = clock.split();

        // Convert frame time from milliseconds to microseconds
        int64_t frame_time_us = GM.getFrameTime() * 1000;

        // Calculate sleep time (need to ensure it's not negative)
        sleep_time = frame_time_us - elapsed_time;

        // Only sleep if positive sleep time
        if (sleep_time > 0) {
            // Convert microseconds to milliseconds for sleep
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
        }
        else {
            // If we're behind, log that we're not keeping up
            LM.writeLog("GameManager::run() - Frame running behind: %lld us", -sleep_time);
        }
    }

    // Cleanup
    LM.writeLog("Cleaning up resources");

    // Shut down InputManager
    IM.shutDown();

    // Terminate GLFW
    glfwTerminate();

    // Properly shut down the GameManager (which will also shut down all other managers)
    GM.shutDown();

    return 0;
}