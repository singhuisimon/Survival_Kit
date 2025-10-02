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
#include "../Manager/SerialisationManager.h"

int main(void) {
    //// Initialize GameManager
    //if (GM.startUp()) {
    //    // Failed to start GameManager
    //    printf("ERROR: Failed to start GameManager\n");
    //    return -1;
    //}

    ZoneScopedN("MainStartup");
	
    bool spacePressed = false;
    bool tracyKeyWasDown = false;

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

    gam300::AssetManager::Config cfg = AM.createDefaultConfig();

    AM.setConfig(cfg);
    AM.startUp();

    AM.scanAndProcess();

    std::cout << "\nFinal database count: " << AM.db().Count() << std::endl;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    IMGUIM.startUp(window, io);

    IMGUIM.initializeAssetManager(); // This will properly initialize the Asset Manager for the Asset Browser

    TRACY.setTracyPath("tracy-profiler.exe");
    TRACY.startUp();

    //bool test_done = false;
    // Create a clock for timing
    gam300::Clock clock;

    // Variables for timing
    int64_t elapsed_time = 0;
    int64_t sleep_time = 0;
    // Main game loop
    LM.writeLog("Starting main game loop");


    //Core::Application app;
    //app.Run();
    //Core::Application app;
    //app.InitializeScripting();
    //app.AddScript(0, "TestScript");

    int frame_count = 0;
    double fps_timer = 0.0;
    double current_fps = 0.0;
    char window_title[256];

    //std::cout << "Initial script added" << std::endl;


    while (!GM.getGameOver() && !glfwWindowShouldClose(window)) {

        ZoneScopedN("MainLoop");

        // Update input system
        IM.update();

        // Start of loop timing
        clock.delta();

        bool currentSpaceState = GetKeyState(VK_SPACE) & 0x8000;
        if (currentSpaceState && !spacePressed)
        {
            //app.ReloadScripts();
            //std::cout << "your mother flip ";
            //app.CreateMonoBehaviourScript("PlayerController");
        }
        // Update all systems (including InputSystem)
        EM.updateSystems(GM.getFrameTime() / 1000.0f);

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        IMGUIM.startImguiFrame();

        IMGUIM.displayTopMenu();


        // Editor Dockspace
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport()); //not working for some reason 

        IMGUIM.renderViewport();

        // Editor Temporary Windows
        IMGUIM.displayPropertiesList();

        IMGUIM.displayHierarchyList();

        IMGUIM.displayAssetsBrowserList();

        IMGUIM.displayPerformanceProfile();

        // Editor Start Render
        ImGui::Render();

        IMGUIM.getWindowSize(*window);
        //std::cout << IMGUIM.getWindowSize(*window).x << "\n";

        IMGUIM.finishImguiRender(io);

        GFXM.update();

        // Swap buffers
        glfwSwapBuffers(window);

        // End of loop timing
        elapsed_time = clock.split();

        frame_count++;
        fps_timer += elapsed_time / 1000000.0;

        if (fps_timer >= 0.1) {
            current_fps = frame_count / fps_timer;

            sprintf_s(window_title, sizeof(window_title),
                "Survival_Kit :: FPS: %.1f :: Frame: %.2fms",
                current_fps, (fps_timer / frame_count) * 1000.0);

            glfwSetWindowTitle(window, window_title);

            frame_count = 0;
            fps_timer = 0.0;
        }

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

        FrameMark; //always keep this as the last.
    }


    //app.ShutdownScripting();
    // Cleanup
    LM.writeLog("Cleaning up resources");

    // Shut down InputManager
    IM.shutDown();
    
    TRACY.shutDown();

    IMGUIM.shutDown();

    AM.shutDown(); //shut down Asset Manager
    
    // Terminate GLFW
    glfwTerminate();

    // Properly shut down the GameManager (which will also shut down all other managers)
    GM.shutDown();

    return 0;
}