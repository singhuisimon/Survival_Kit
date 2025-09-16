/**
 * @file ImguiManager.cpp
 * @brief Implementation of the functions of IMGUI_Manager class for running the IMGUI level editor.
 * @author Liliana Hanawardani, Saw Hui Shan
 * @date September 8, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
 
 // Include header file
#include "ImguiManager.h"
#include "ECSManager.h"
#include "SerialisationManager.h"
#include <iostream>
#include "../Utility/AssetPath.h"
#include "../Component/Transform3D.h"

namespace gam300 {

    static bool hierachyWindow = true;
    static bool inspectorWindow = true;
    static bool assetsBrowser = true; // to load assets

    // Entity index
    int selectedObjIndex = -1;

    ImguiManager::ImguiManager() : ImguiEcsRef(EM) {}

    ImguiManager::ImguiManager(ECSManager& ECS) : ImguiEcsRef(ECS) {
        setType("IMGUI_Manager");
    }

    ImguiManager& ImguiManager::getInstance() {
        static ImguiManager instance;
        return instance;
    }

    //Inherited start_up function, not using
    int ImguiManager::startUp() {
        throw std::runtime_error("No-parameter start_up() is disabled in IMGUI_Manager. start_up() now has a parameter GLFWwindow*& window");
    }

    //Start up function
    int ImguiManager::startUp(GLFWwindow*& glfwindow, ImGuiIO& imgui_io) {
        if (isStarted()) {
            LM.writeLog("IMGUI_Manager::start_up(): Already started.");
            return 0; // Already started
        }

        imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        imgui_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
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
        if (imgui_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(glfwindow, true);
        ImGui_ImplOpenGL3_Init();

        return 0;
    }

    void ImguiManager::startImguiFrame() {

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    }

    void ImguiManager::finishImguiRender(ImGuiIO& imgui_io) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (imgui_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImguiManager::displayFileList(bool& fileWindow, std::string& shownFile) {

        std::string scenePath = getAssetFilePath("Scene");
        std::vector<std::pair<std::string, std::string>> sceneFiles;

        if (std::filesystem::exists(scenePath) && std::filesystem::is_directory(scenePath))
        {
            for (const auto& file : std::filesystem::directory_iterator(scenePath)) {
                if (std::filesystem::is_regular_file(file.path())) {
                    sceneFiles.push_back(std::make_pair(file.path().filename().string(), file.path().string()));
                }
            }
        }

        ImGui::SetNextWindowSize(ImVec2(800, 400));

        if (ImGui::Begin("Level Select", &fileWindow, ImGuiWindowFlags_NoDocking)) {
            for (int i = 0; i < sceneFiles.size(); i++)
            {
                std::string fileName = sceneFiles[i].first;
                if (ImGui::Selectable(fileName.c_str())) {

                    if (sceneFiles[i].second != shownFile) {

                        if (SEM.loadScene(sceneFiles[i].second)) {

                            shownFile = sceneFiles[i].second;

                            LM.writeLog("IMGUI_Manager::displayFileList(): Scene %s loaded successfully.", sceneFiles[i].first.c_str());
                            //std::cout << sceneFiles[i].second << std::endl;
                            //std::cout << "Scene " << sceneFiles[i].first << "loaded successfully from displayFileList" << std::endl;

                        }
                        else {

                            LM.writeLog("IMGUI_Manager::displayFileList(): Scene %s failed to load. Loading default scene.", sceneFiles[i].first.c_str());
                            //std::cout << "Scene " << sceneFiles[i].first << "failed to load from displayFileList. Loading default scene." << std::endl;

                            SEM.saveScene(getAssetFilePath("Scene/Game.scn"));
                            if (SEM.loadScene(getAssetFilePath("Scene/Game.scn"))) {

                                LM.writeLog("IMGUI_Manager::displayFileList(): Default scene loaded successfully.");
                                //std::cout << "Default scene loaded successfully from displayFileList" << std::endl;
                            }
                            else {

                                LM.writeLog("IMGUI_Manager::displayFileList(): WARNING: Failed to load default scene.");
                                //std::cout << "WARNING: Failed to load default scene from displayFileList" << std::endl;
                            }

                            shownFile = getAssetFilePath("Scene/Game.scn");
                        }
                    }
                    else {

                        //std::cout << "Scene " << sceneFiles[i].first << " is already loaded." << std::endl;

                        LM.writeLog("Scene %s is already loaded.", sceneFiles[i].first.c_str());
                        
                        //std::cout << "shownFile: " << shownFile << std::endl;
                        //std::cout << "sceneFiles[i].second: " << sceneFiles[i].second << std::endl;

                    }

                    fileWindow = false;
                    ImGui::CloseCurrentPopup();
                    break;
                }
            }

            ImGui::End();
        }
        
    }

    void ImguiManager::displayHierarchyList() {
 
        ImGui::SetNextWindowSize(ImVec2(600, 400));

        if (ImGui::Begin("Hierarchy", &hierachyWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
            const auto& allEntities = ImguiEcsRef.getAllEntities();

            // Debug code
            // std::cout << "Entity count: " << allEntities.size() << "\n";

            /*  for (auto& e_name : allEntities)
                {
                    std::cout << "Entity Name: " << e_name.get_name() << "\n";
                }
           */
            //int currObjIndex = -1;
            
            //Toggle Select object
            //std::cout << "Check entities list from Imgui: " << allEntities.size() << std::endl;

            if (allEntities.empty())
            {
                //std::cout << "There is no entities to get in IMGUI\n"; 
                ImGui::Text("No entity available.");
                selectedObjIndex = -1; // reset selected object if there is no entity to get
            }
            else
            {
                //std::cout << "There are entities to get in IMGUI\n";
                for (int i = 0; i < allEntities.size(); i++)
                {
                    const std::string objName = allEntities[i].get_name();
                    if (ImGui::Selectable(objName.c_str(), selectedObjIndex == i))
                    {
                        selectedObjIndex = i;
                    }

                    //++currObjIndex;
                }
            }
           
        }

        ImGui::End();
    }

    void ImguiManager::displayPropertiesList()
    {
        ImGui::SetWindowSize(ImVec2(600, 400));
        if (ImGui::Begin("Properties/ Inspector", &inspectorWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {

        }

        // Get all entities
        auto& allEntities = ImguiEcsRef.getAllEntitiesOverload();

        if (selectedObjIndex >= allEntities.size())
        {
            ImGui::Text("No Entity Available, Invalid Selection");
            selectedObjIndex = -1;
        
        }
        else
        {
            // get the selected entity from list
            auto& selectedEntity = allEntities[selectedObjIndex];

            // display information of the entity info
            //ImGui::Text("Entity Name: %s", selectedEntity.get_name().c_str());

            // displlay information using text input
            char nameBuffer[128];
            strncpy_s(nameBuffer, selectedEntity.get_name().c_str(), sizeof(nameBuffer));
            if (ImGui::InputText("Entity Name", nameBuffer, sizeof(nameBuffer)))
            {
                const std::string newName = nameBuffer;
                selectedEntity.set_name(newName);
            }
           

          
        }
        ImGui::End();

    }

    void ImguiManager::shutDown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

}// end of namespace gam300