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
        // Failed to start GameManager
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
    GLFWwindow* window = glfwCreateWindow(640, 480, "Survival_Kit", NULL, NULL);
    if (!window) {
        LM.writeLog("ERROR: Failed to create GLFW window");
        glfwTerminate();
        GM.shutDown();
        return -1;
    }

    LM.writeLog("Window created with dimensions 640x480");
    glfwMakeContextCurrent(window);

    // Register window with InputManager
    IM.setWindow(window);
    LM.writeLog("InputManager initialized successfully");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    //style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    //style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    bool test_window = true;
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

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(600, 400));
        if (ImGui::Begin("Test", &test_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {


        }
        ImGui::End();

        ImGui::Render();

        // Render frame 
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Terminate GLFW
    glfwTerminate();

    // Properly shut down the GameManager (which will also shut down all other managers)
    GM.shutDown();

    return 0;
}