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
#include "SerialisationBinManager.h"

#include "../Manager/PrefabManager.h"
#include "../Manager/AssetManager.h"

#include "../Utility/AssetPath.h"

#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include "../Component/Collider.h"
#include "../Component/AudioComponent.h"
#include "../Component/MeshComponent.h"

#include <iostream>

namespace gam300 {

    static bool hierachyWindow = true;
    static bool inspectorWindow = true;
    static bool assetsBrowser = true; // to load assets

    // ----------------- ASSET BROWSER FUNCTIONALITY -----------------

    // Asset Browser States
    static std::vector<const AssetRecord*> filteredAssets;
    static std::string searchFilter = "";
    static AssetType selectedAssetType = AssetType::Unknown;    //filter by asset type
    static bool needsRefresh = true;
    static float assetIconSize = 64.0f;
    static int selectedAssetIndex = -1;

    static bool asset_editor = false;
    static bool prefab_editor = false;
    static std::filesystem::directory_entry currentAsset;
    static std::filesystem::directory_entry selectedPrefab;

    // Asset Type names for display
    const char* ImguiManager::getAssetTypeName(AssetType type)
    {
        switch (type)
        {
        case AssetType::Texture: return "Texture";
        case AssetType::Audio: return "Audio";
        case AssetType::Mesh: return "Mesh";
        case AssetType::Shader: return "Shader";
        case AssetType::Material: return "Material";
        case AssetType::Scene: return "Scene";
        case AssetType::Unknown:
        default: return "Unknown";
        }
    }

    // get Asset icon based on type
    const char* ImguiManager::getAssetIcon(AssetType type)
    {
        switch (type) {
        case AssetType::Texture: return "[IMG]";
        case AssetType::Audio: return "[SND]";
        case AssetType::Mesh: return "[3D]";
        case AssetType::Shader: return "[SHD]";
        case AssetType::Material: return "[MAT]";
        case AssetType::Scene: return "[SCN]";
        case AssetType::Unknown:
        default: return "[?]";
        }
    }

    // Entity index
    int selectedObjIndex = -1;

    //Prefab counter index
    static int counter = 0;

    ImguiManager::ImguiManager() : ImguiEcsRef(EM), ImguiGraphicRef(GFXM) {}

    ImguiManager::ImguiManager(ECSManager& ECS, GraphicsManager& GFM) : ImguiEcsRef(ECS), ImguiGraphicRef(GFM) {
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

#if 1 // new code
        if (fileWindow)
        {
            ImGui::OpenPopup("Level Select");
            //std::cout << "is open here\n";

        }

        if (ImGui::BeginPopupModal("Level Select", nullptr, ImGuiWindowFlags_NoDocking))
        {
            //std::cout << "is open here\n";
            ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);

            for (int i = 0; i < sceneFiles.size(); i++)
            {
                std::string fileName = sceneFiles[i].first;
                if (ImGui::Selectable(fileName.c_str())) {

                    ImguiEcsRef.clearAllEntities();

                    //if (sceneFiles[i].second != shownFile) {

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
                    fileWindow = false; // reset
                    ImGui::CloseCurrentPopup();
                    break;
                }
            }
            if (ImGui::Button("Cancel")) {
                fileWindow = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
#endif 

    }

    static std::string prefab_name;
    static int prefab_index = -1;

    void ImguiManager::displayHierarchyList() {

        ImGui::SetNextWindowSize(ImVec2(600, 400));

        if (ImGui::Begin("Hierarchy", &hierachyWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
            const auto& allEntities = ImguiEcsRef.getAllEntities();

            // Add new entity
            if (ImGui::Button("+"))
            {
                ImGui::OpenPopup("New Action");
            }

            if (ImGui::BeginPopup("New Action"))
            {
                if (ImGui::MenuItem("Add Entity"))
                {
                    Entity& createNewEntity = ImguiEcsRef.createEntity("New Entity");

                    // always add default transform3D
                    if (!ImguiEcsRef.hasComponent<Transform3D>(createNewEntity.get_id())) {
                        ImguiEcsRef.addComponent<Transform3D>(createNewEntity.get_id());
                    }

                    // always show the new entity that is create 
                    selectedObjIndex = static_cast<int>(allEntities.size()) - 1;
                }


                ImGui::EndPopup();
            }


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

                    // delete entity by right click 
                    if (ImGui::BeginPopupContextItem(("Entity Context" + std::to_string(i)).c_str()))
                        //if (ImGui::BeginPopup("Selected Entity Menu"))
                    {
                        if (ImGui::MenuItem("Delete"))
                        {
                            if (selectedObjIndex >= 0 && selectedObjIndex < static_cast<int>(allEntities.size()))
                            {
                                EntityID idToDelete = allEntities[selectedObjIndex].get_id();
                                ImguiEcsRef.destroyEntity(idToDelete);

                                selectedObjIndex = -1;
                            }
                        }
                        if (ImGui::MenuItem("Duplicate"))
                        {
                            if (selectedObjIndex >= 0 && selectedObjIndex < static_cast<int>(allEntities.size()))
                            {
                                // to duplicate entity name
                                std::string idToDuplicate = allEntities[selectedObjIndex].get_name();
                                Entity& duplicatedEntity = ImguiEcsRef.createEntity(idToDuplicate);


                                // get information for original entity 
                                const Entity& oriSelectedEntity = allEntities[selectedObjIndex];
                                EntityID newEntityID = duplicatedEntity.get_id();

                                if (ImguiEcsRef.hasComponent<Transform3D>(oriSelectedEntity.get_id()))
                                {
                                    Transform3D* oldTransform = ImguiEcsRef.getComponent<Transform3D>(oriSelectedEntity.get_id());
                                    if (oldTransform)
                                    {

                                        ImguiEcsRef.addComponent<Transform3D>(newEntityID, *oldTransform);
                                    }
                                }

                                if (ImguiEcsRef.hasComponent<RigidBody>(oriSelectedEntity.get_id()))
                                {
                                    RigidBody* oldRigidbody = ImguiEcsRef.getComponent<RigidBody>(oriSelectedEntity.get_id());
                                    if (oldRigidbody)
                                    {

                                        ImguiEcsRef.addComponent<RigidBody>(newEntityID, *oldRigidbody);
                                    }
                                }

                                if (ImguiEcsRef.hasComponent<Collider>(oriSelectedEntity.get_id()))
                                {
                                    Collider* oldRigidbody = ImguiEcsRef.getComponent<Collider>(oriSelectedEntity.get_id());
                                    if (oldRigidbody)
                                    {

                                        ImguiEcsRef.addComponent<Collider>(newEntityID, *oldRigidbody);
                                    }
                                }
                                if (ImguiEcsRef.hasComponent<AudioComponent>(oriSelectedEntity.get_id()))
                                {
                                    AudioComponent* oldAudio = ImguiEcsRef.getComponent<AudioComponent>(oriSelectedEntity.get_id());
                                    if (oldAudio)
                                    {

                                        ImguiEcsRef.addComponent<AudioComponent>(newEntityID, *oldAudio);
                                    }
                                }
                                if (ImguiEcsRef.hasComponent<MeshComponent>(oriSelectedEntity.get_id()))
                                {
                                    MeshComponent* oldMesh = ImguiEcsRef.getComponent<MeshComponent>(oriSelectedEntity.get_id());
                                    if (oldMesh)
                                    {

                                        ImguiEcsRef.addComponent<MeshComponent>(newEntityID, *oldMesh);
                                    }
                                }
                                selectedObjIndex = static_cast<int>(allEntities.size()) - 1;

                            }
                        }
                        ImGui::Separator();
                        if (ImGui::BeginMenu("Prefabs"))
                        {
                            if (ImGui::MenuItem("Create Prefabs"))
                            {
                                prefab_index = i;
                                showPrefabPanel = true; // to open pop up for the prefabs

                            }

                            ImGui::Separator();
                            if (ImGui::MenuItem("Replace Prefabs"))
                            {

                            }

                            ImGui::EndMenu();
                        }

                        ImGui::EndPopup();
                    }

                    //++currObjIndex;
                }

                if (showPrefabPanel && prefab_index >= 0 && prefab_index < static_cast<int>(allEntities.size()))
                {
                    const Entity& prefabSelectedEntity = allEntities[prefab_index];
                    prefab_name = prefabSelectedEntity.get_name();
                    if (PM.createPrefabFromEntity(prefabSelectedEntity.get_id(), prefab_name, true)) {
                        ImGui::OpenPopup("Create Prefab Panel");
                    }

                    showPrefabPanel = false;

                }

                if (ImGui::BeginPopupModal("Create Prefab Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("%s has been created.", prefab_name.c_str());

                    if (ImGui::Button("Close"))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }

            }

        }

        ImGui::End();
    }




#if 1
    void ImguiManager::displayPropertiesList()
    {
        ImGui::SetNextWindowSize(ImVec2(600, 400));



        if (ImGui::Begin("Properties/ Inspector", &inspectorWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            // to get all the entities 
            const std::vector<Entity>& allEntities = ImguiEcsRef.getAllEntities();
            if (allEntities.empty() || selectedObjIndex < 0 || selectedObjIndex >= static_cast<int>(allEntities.size()))
            {
                selectedObjIndex = -1;

            }
            else
            {

                // Get the selected entity (Fix: proper array access)
                const Entity& selectedEntity = allEntities[selectedObjIndex];

                // Display entity information using input text (Fix: strncpy_s parameters)
                char nameBuffer[128];
                const std::string& selectedEntityName = selectedEntity.get_name();

                // Fix: strncpy_s requires 3 parameters: destination, size, source
                strcpy_s(nameBuffer, sizeof(nameBuffer), selectedEntityName.c_str());

                // add ImGuiInputTextFlags_EnterReturnsTrue to ensure only change name after user press enter
                // Fix: Game crash if delete the last alphabet since it keep updating the frame and cause a empty ID 
                if (ImGui::InputText("Entity Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    std::string newSelectedEntityName = nameBuffer;

                    if (newSelectedEntityName.empty()) {
                        newSelectedEntityName = selectedEntity.get_name();
                    }

                    // Fix: Can't modify const entity directly, need to use ECS manager
                    ImguiEcsRef.renameEntity(selectedEntity.get_id(), newSelectedEntityName);
                }

                // Display entity ID
                ImGui::Text("Entity ID: %u", selectedEntity.get_id());
                // Display component information
                ImGui::Separator();
                ImGui::Text("Components:");

                // Display adjustable value in components  
                if (ImguiEcsRef.hasComponent<Transform3D>(selectedEntity.get_id())) {
                    displayComponentMenu<Transform3D>(selectedEntity.get_id(), "Transform3D");

                }
                if (ImguiEcsRef.hasComponent<RigidBody>(selectedEntity.get_id())) {
                    displayComponentMenu<RigidBody>(selectedEntity.get_id(), "RigidBody");
                    //displayComponentMenu<RigidBody>(selectedEntity.get_id(), "RigidBody");

                }
                if (ImguiEcsRef.hasComponent<Collider>(selectedEntity.get_id())) {
                    displayComponentMenu<Collider>(selectedEntity.get_id(), "Collider");
                    //displayComponentMenu<RigidBody>(selectedEntity.get_id(), "RigidBody");

                }
                if (ImguiEcsRef.hasComponent<AudioComponent>(selectedEntity.get_id())) {
                    displayComponentMenu<AudioComponent>(selectedEntity.get_id(), "Audio");
                    //displayComponentMenu<RigidBody>(selectedEntity.get_id(), "RigidBody");

                }



                ImGui::Separator();
                // Adding components 

                ImVec2 windowSize = ImGui::GetWindowSize(); // get Properties window size
                ImVec2 buttonSize(140, 40); // set button size

                // Calculate centered position for x axis
                ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);

                if (ImGui::Button("Add Component", buttonSize))
                {
                    //ImGui::Text("Component"); // for now only transform3D component

                    ImGui::OpenPopup("AddComponentPopup");
                }
                if (ImGui::BeginPopup("AddComponentPopup")) {
                    if (ImGui::MenuItem("Transform3D")) {
                        if (!ImguiEcsRef.hasComponent<Transform3D>(selectedEntity.get_id())) {
                            ImguiEcsRef.addComponent<Transform3D>(selectedEntity.get_id());
                        }
                    }
                    if (ImGui::MenuItem("RigidBody")) {
                        if (!ImguiEcsRef.hasComponent<RigidBody>(selectedEntity.get_id())) {
                            ImguiEcsRef.addComponent<RigidBody>(selectedEntity.get_id());
                        }
                    }
                    if (ImGui::MenuItem("Collider"))
                    {
                        if (!ImguiEcsRef.hasComponent<Collider>(selectedEntity.get_id()))
                        {
                            ImguiEcsRef.addComponent<Collider>(selectedEntity.get_id());
                        }
                    }
                    if (ImGui::MenuItem("Audio"))
                    {
                        if (!ImguiEcsRef.hasComponent<AudioComponent>(selectedEntity.get_id()))
                        {
                            ImguiEcsRef.addComponent<AudioComponent>(selectedEntity.get_id());
                        }
                    }
                    if (ImGui::MenuItem("Mesh"))
                    {
                        if (!ImguiEcsRef.hasComponent<MeshComponent>(selectedEntity.get_id()))
                        {
                            ImguiEcsRef.addComponent<MeshComponent>(selectedEntity.get_id());
                        }
                    }

                    ImGui::EndPopup();
                }
                // Removed Component 


            }
        }

        ImGui::End();
    }
#endif

    void ImguiManager::showPrefabsPanel(EntityID selectedEntity)
    {
        //if (ImGui::Begin)
    }

    void ImguiManager::displayTopMenu()
    {
        //static std::string shownFile = getAssetFilePath("Scene/") + "default.scn";


        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Separator();

            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                    std::string sceneFolder = getAssetFilePath("Scene");

                }

                if (ImGui::MenuItem("Open"))
                {
                    fileWindow = true;


                }
                ImGui::Separator();
                if (ImGui::MenuItem("Save"))
                {

                    //To uncomment after Serialisation is fixed
                    SEM.saveScene(shownFile);
                    std::cout << shownFile << "\n";

                }
                if (ImGui::MenuItem("Save as"))
                {
                    showSaveAsPanel = true;

                }
                ImGui::EndMenu();
                ImGui::Separator();

            }
            //ImGui::SameLine(); // make it appear on the same bar

            std::string displayedScene = shownFile;
            size_t pos = shownFile.find("Assets"); // find "Assets" in the path
            if (pos != std::string::npos) {
                displayedScene = shownFile.substr(pos);
            }

            // Get the window width and text width
            float windowWidth = ImGui::GetWindowWidth();
            float textWidth = ImGui::CalcTextSize(displayedScene.c_str()).x;

            // Move cursor to the right
            ImGui::SetCursorPosX(windowWidth - textWidth - 10); // 10 pixels padding from right
            ImGui::Text("%s", displayedScene.c_str());

            //ImGui::Text("Current Scene: %s", displayedScene.c_str());

            ImGui::EndMainMenuBar();

        }
        //ImGui::Text("Current Scene: %s", shownFile.c_str());
        //ImGui::Begin("Current Scene");

        //ImGui::End();

        if (fileWindow) {

            IMGUIM.displayFileList(fileWindow, shownFile); // for now it open at the start of the engine
        }

        std::string newPath = getAssetFilePath("Scene/") + saveAsDefaultName;

        if (showSaveAsPanel)
        {
            ImGui::OpenPopup("Save Scene As");

            if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::InputText("File name", saveAsDefaultName, IM_ARRAYSIZE(saveAsDefaultName));

                if (ImGui::Button("Save", ImVec2(120, 0)))
                {

                    if (!std::filesystem::path(newPath).has_extension()) {
                        newPath += ".scn"; // ensure .scn extension
                    }

                    if (std::filesystem::exists(newPath))
                    {
                        ImGui::OpenPopup("Confirm Overwrite");
                    }
                    else
                    {
                        SEM.saveScene(newPath);
                        shownFile = newPath; // update current scene
                        LM.writeLog("Scene saved as: %s", newPath.c_str());
                        showSaveAsPanel = false;
                        ImGui::CloseCurrentPopup();

                    }


                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    showSaveAsPanel = false;
                    ImGui::CloseCurrentPopup();
                }

                // overwrite 
                if (ImGui::BeginPopupModal("Confirm Overwrite", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("File %s already exists.\nDo you want to replace it?", saveAsDefaultName);
                    ImGui::Separator();

                    if (ImGui::Button("Yes", ImVec2(120, 0)))
                    {

                        if (!std::filesystem::path(newPath).has_extension()) {
                            newPath += ".scn";
                        }

                        SEM.saveScene(newPath);
                        shownFile = newPath;

                        showSaveAsPanel = false;
                        ImGui::CloseCurrentPopup(); // close Save As
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("No", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                ImGui::EndPopup();
            }
        }


    }

    void ImguiManager::shutDown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    Vector2D ImguiManager::getWindowSize(GLFWwindow& window)
    {
        //Vector2D dimension{ 0,0 };

        glfwGetWindowSize(&window, &width, &height);
        return Vector2D(width, height);
        //std::cout << width << ", " << "height\n";
    }

    void ImguiManager::renderViewport()
    {
        auto texture = GFXM.getImguiTex();
        ImVec2 texture_pos = ImGui::GetCursorScreenPos();

        ImVec2 viewportSize =
        {
             static_cast<float>(getWindowWidthHeight().x) / 2.0f,
             static_cast<float>(getWindowWidthHeight().y) / 2.0f
        };
        /* int windowWidth = getWindowWidthHeight().x / 2;
         int windowHeight = getWindowWidthHeight().y / 2;*/

        ImGui::Begin("Viewport");

        if (texture) {

            ImVec2 imagePos = ImGui::GetCursorScreenPos();

            ImGui::Image((ImTextureID)(intptr_t)GFXM.getImguiTex(),
                viewportSize,
                ImVec2(0, 1), ImVec2(1, 0));

            handleViewPortClick(imagePos, viewportSize);
        }

        //ImVec2 mousePos, viewportSize;
        //void(mousePos);

        ImGui::End();
    }

    // ASSET BROWSER FUNCTIONS TO BE PLACED HERE
#if 1
    void ImguiManager::refreshAssetList()
    {
        filteredAssets.clear();

        //get all assets from the Asset Manager Database
        auto allAssets = AM.db().AllMutable();

        for (auto* asset : allAssets)
        {
            if (!asset || !asset->valid) continue;  //skip invalid

            //apply asset type filter
            if (selectedAssetType != AssetType::Unknown && asset->type != selectedAssetType) {
                continue;
            }

            // apply search filter
            if (!searchFilter.empty()) {
                std::string assetName = std::filesystem::path(asset->sourcePath).filename().string();
                std::string lowerAssetName = assetName;
                std::string lowerFilter = searchFilter;

                // Convert to lowercase for case-insensitive search
                std::transform(lowerAssetName.begin(), lowerAssetName.end(), lowerAssetName.begin(), ::tolower);
                std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

                if (lowerAssetName.find(lowerFilter) == std::string::npos) {
                    continue;
                }
            }

            filteredAssets.push_back(asset);
        }

        //sort assets alphabetically by filename
        std::sort(filteredAssets.begin(), filteredAssets.end(),
            [](const AssetRecord* a, const AssetRecord* b) {
                std::string nameA = std::filesystem::path(a->sourcePath).filename().string();
                std::string nameB = std::filesystem::path(b->sourcePath).filename().string();
                return nameA < nameB;
            });

        needsRefresh = false;
    }

#endif

    void ImguiManager::initializeAssetBrowser() {
        // Make sure Asset Manager is started up
        if (!AM.isStarted()) {
            LM.writeLog("ERROR: Asset Manager not initialized before IMGUI!");
            return;
        }
        // Set flag to refresh the browser
        needsRefresh = true;

        LM.writeLog("Asset Browser: Initialized and scanning assets...");
    }

    void ImguiManager::initializeAssetManager() {
        initializeAssetBrowser();
    }

    void ImguiManager::rescanAssets() {
        AM.scanAndProcess();
        needsRefresh = true;
        LM.writeLog("Asset Browser: Manual asset rescan triggered");
    }

#if 1// new code
    void ImguiManager::displayAssetsBrowserList()
    {
        // --- Window setup ---//
        ImGui::SetNextWindowSize(ImVec2(600, 400));
        if (!ImGui::Begin("Assets Browser", &inspectorWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            ImGui::End();
            return;
        }

        // --- Search bar ---//
        static char searchBuffer[256] = "";
        ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));
        std::string searchStr(searchBuffer);
        std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

        // --- load folders ---//
        std::vector<std::string> assetsFoldersName;
        if (std::filesystem::exists(BASE_ASSETS_PATH) && std::filesystem::is_directory(BASE_ASSETS_PATH))
        {
            for (const auto& entry : std::filesystem::directory_iterator(BASE_ASSETS_PATH))
            {
                if (entry.is_directory())
                    assetsFoldersName.push_back(entry.path().filename().string());
            }
        }
        else
        {
            ImGui::Text("No Folder Exist");
        }

        static std::string selectedFolder = ""; // currently selected folder type e.g. scene, prefab
        static int selectedAssetIndex = -1;
        //std::cout << "selectedFolder: " << selectedFolder << "\n";
        // ---set up 2 column ---
        ImGui::Columns(2, nullptr, true);

        // ----- Left column panel:  ----- //
        ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Projects:");
        if (ImGui::CollapsingHeader("Assets", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (const auto& folder : assetsFoldersName)
            {
                bool isSelected = (selectedFolder == folder);
                if (ImGui::Selectable(folder.c_str(), isSelected))
                {
                    selectedFolder = folder;
                    selectedAssetIndex = -1; // reset asset selection
                }
            }
        }
        ImGui::EndChild();

        // ----- rigt column panel: Assets ----- // 
        ImGui::NextColumn();
        ImGui::BeginChild("Assets Panel", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        std::vector<std::filesystem::directory_entry> assetsList;

        // --- display assets ---
        if (!selectedFolder.empty())
        {
            // to allow search  funcion
            if (!searchStr.empty())
            {
                // Search across all folders
                for (const auto& folderEntry : std::filesystem::directory_iterator(BASE_ASSETS_PATH))
                {
                    if (!folderEntry.is_directory()) continue;

                    for (const auto& fileEntry : std::filesystem::directory_iterator(folderEntry.path()))
                    {
                        if (!fileEntry.is_regular_file()) continue;

                        std::string filename = fileEntry.path().filename().string();
                        std::string lowerName = filename;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

                        if (lowerName.find(searchStr) != std::string::npos)
                            assetsList.push_back(fileEntry);
                    }
                }
            }
            else
            {
                // normal view: only selected folder
                std::string currentFolder = BASE_ASSETS_PATH + selectedFolder + "\\";
                if (std::filesystem::exists(currentFolder) && std::filesystem::is_directory(currentFolder))
                {
                    for (const auto& entry : std::filesystem::directory_iterator(currentFolder))
                    {
                        if (entry.is_regular_file())
                            assetsList.push_back(entry);
                    }
                }


            }
        }



        // --- display assets grid ---
        if (!assetsList.empty())
        {
            ImGui::Text(("Assets > " + selectedFolder).c_str()); // to show which folder we in
            ImGui::Separator();

            float padding = 10.0f;
            float thumbnailSize = 64.0f;
            float cellSize = thumbnailSize + padding;
            float panelWidth = ImGui::GetContentRegionAvail().x;
            int itemsPerRow = std::max(1, (int)(panelWidth / cellSize));

            ImGui::Columns(itemsPerRow, nullptr, false);
            for (size_t i = 0; i < assetsList.size(); ++i)
            {
                const auto& assetEntry = assetsList[i];
                std::string filename = assetEntry.path().filename().string();
                std::string fileNamePath = assetEntry.path().string();

                ImGui::PushID(filename.c_str());
                if (ImGui::Button(filename.c_str(), ImVec2(thumbnailSize, thumbnailSize))) {


                    selectedAssetIndex = i;
                    shownFile = fileNamePath;
                    // to ge the type of the file e.g. .scn, .wav
                    std::string extension = assetEntry.path().extension().string();


                    //std::cout << extension << "\n";
                    if (extension == ".scn") // if it is scene
                    {
                        ImguiEcsRef.clearAllEntities();
                        SEM.loadScene(assetEntry.path().string());
                        shownFile = fileNamePath;

                    }
                    if (extension == ".png" || extension == ".jpeg") //to open the image
                    {
                        asset_editor = true;
                        currentAsset = assetEntry;
                    }
                    if (extension == ".prefab") //to open the image
                    {
                        prefab_editor = true;
                        selectedPrefab = assetEntry;
                    }
                }

                // ------- show toolip detail ---------- 
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Name: %s", filename.c_str());
                    ImGui::Text("Type: %s", assetEntry.path().extension().string().c_str());
                    ImGui::Text("Path: %s", assetEntry.path().string().c_str());

                    std::string intermediatePath = "";
                    if (!intermediatePath.empty())
                    {
                        ImGui::Text("Intermediate: %s", intermediatePath.c_str());

                    }


                    auto ftime = std::filesystem::last_write_time(assetEntry.path());

                    // convert to system_clock time_point
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
                    );

                    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

                    char timeBuffer[64];
                    ctime_s(timeBuffer, sizeof(timeBuffer), &tt);

                    // remove newline added by ctime_s
                    if (strlen(timeBuffer) > 0 && timeBuffer[strlen(timeBuffer) - 1] == '\n')
                        timeBuffer[strlen(timeBuffer) - 1] = '\0';

                    ImGui::Text("Last Modified: %s", timeBuffer);

                    ImGui::EndTooltip();
                }

                // Center text under thumbnail
                ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
                float textX = (thumbnailSize - textSize.x) * 0.5f;
                if (textX < 0) textX = 0;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
                ImGui::TextWrapped("%s", filename.c_str());

                ImGui::NextColumn();
                ImGui::PopID();
            }
            ImGui::Columns(1);

            if (asset_editor) {
                displayAssetEditor(currentAsset);
            }

            if (prefab_editor) {
                displayPrefabEditor(selectedPrefab);
            }
        }

        ImGui::EndChild();
        ImGui::Columns(1);
        ImGui::End();
    }


#endif

    void ImguiManager::displayAssetEditor(const std::filesystem::directory_entry& assetFilepath)
    {
        if (ImGui::Begin("Asset Editor", &asset_editor)) {
            ImGui::Text("Editing: %s", assetFilepath.path().string().c_str());
        }
        ImGui::End();

        if (!asset_editor) {
            currentAsset = std::filesystem::directory_entry();
        }
    }

    void ImguiManager::displayPrefabEditor(const std::filesystem::directory_entry& prefabFilepath)
    {
        if (ImGui::Begin("Prefab Editor", &prefab_editor)) {
            ImGui::Text("Editing: %s", prefabFilepath.path().string().c_str());
        }
        ImGui::End();

        if (!prefab_editor) {
            selectedPrefab = std::filesystem::directory_entry();
        }
    }

    void ImguiManager::handleViewPortClick(ImVec2 mousePos, ImVec2 viewportSize)
    {
        // Check the mouse hover position for reference
        if (ImGui::IsItemHovered())
        {
            ImVec2 mouseScreen = ImGui::GetIO().MousePos;

            ImVec2 mouseInViewportPos;
            mouseInViewportPos.x = mouseScreen.x - mousePos.x;
            mouseInViewportPos.y = mouseScreen.y - mousePos.y;

            ImGui::Text("Mouse local: (%.1f, %.1f)", mouseInViewportPos.x, mouseInViewportPos.y);
            float aspectRatio = viewportSize.x / viewportSize.y;

        }
    }


    template<typename componentType>
    void ImguiManager::displayComponentMenu(EntityID entityID, const char* componentName)
    {

        //const auto& componentTypes = ImguiEcsRef
        // Create column to split the CollapsingHeader and component menu 
        ImGui::Columns(2, nullptr, false);
        ImGui::PushID(componentName);


        // Left column 
        ImGui::SetColumnWidth(0, 200.0f);
        bool openHeader = ImGui::CollapsingHeader(componentName);

        ImGui::SetItemAllowOverlap(); // allow overlapping hover for button
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("Popup");
        }


        ImGui::NextColumn(); // move to right 

        // Right column
        if (ImGui::Button("...")) {
            //std::cout << "Clicked remove for " << componentName << "\n";
            ImGui::OpenPopup("Popup");
        }

        if (ImGui::BeginPopup("Popup")) {
            if (ImGui::MenuItem("Remove Component")) {
                ImguiEcsRef.removeComponent<componentType>(entityID);
            }
            ImGui::EndPopup();
        }

        ImGui::Columns(1); // reset back to single column

        // display the editable value
        if (openHeader) {
            displayComponentContent<componentType>(entityID);

        }

        ImGui::PopID();
    }

    template<typename componentType>
    void ImguiManager::displayComponentContent(EntityID selectedEntityID)
    {

        // Transform Component
        if constexpr (std::is_same_v<componentType, Transform3D>) {
            if (Transform3D* transform = ImguiEcsRef.getComponent<componentType>(selectedEntityID)) {
                if (transform) {
                    // Position
                    Vector3D pos = transform->getPosition();
                    float position[3] = { pos.x, pos.y, pos.z };
                    if (ImGui::DragFloat3("Position", position, 0.1f)) {
                        transform->setPosition(Vector3D(position[0], position[1], position[2]));
                    }

                    // Rotation
                    Vector3D rot = transform->getRotation();
                    float rotation[3] = { rot.x, rot.y, rot.z };
                    if (ImGui::DragFloat3("Rotation", rotation, 1.0f)) {
                        transform->setRotation(Vector3D(rotation[0], rotation[1], rotation[2]));
                    }

                    // Scale
                    Vector3D scl = transform->getScale();
                    float scale[3] = { scl.x, scl.y, scl.z };
                    if (ImGui::DragFloat3("Scale", scale, 0.1f)) {
                        transform->setScale(Vector3D(scale[0], scale[1], scale[2]));
                    }
                }
            }
        }
        else if constexpr (std::is_same_v<componentType, RigidBody>) {
            // rigidBody 
            if (RigidBody* rigidBody = ImguiEcsRef.getComponent<RigidBody>(selectedEntityID)) {

                if (rigidBody)
                {
                    ImGui::Separator(); // for editable value



                    float mass = rigidBody->getMass();
                    if (ImGui::InputFloat("Mass", &mass)) {
                        rigidBody->setMass(mass);
                    }


                    Vector3D iDiagonal = rigidBody->getInertiaDiagonal();
                    float inertiaDiagonal[3] = { iDiagonal.x, iDiagonal.y, iDiagonal.z };
                    if (ImGui::DragFloat3("Inertia Diagonal", inertiaDiagonal, 0.1f)) {
                        rigidBody->setInertiaDiagonal(Vector3D(inertiaDiagonal[0], inertiaDiagonal[1], inertiaDiagonal[2]));
                    }

                    static bool forceMask = true;
                    if (ImGui::Checkbox("ForceMask", &forceMask)) {
                        rigidBody->setForceMask(forceMask ? 0xFFFFFFFFu : 0u);
                    }

                    static bool torqueMask = true;
                    if (ImGui::Checkbox("Torque Mask", &torqueMask)) {
                        rigidBody->setTorqueMask(torqueMask ? 0xFFFFFFFFu : 0u);
                    }

                    //TODO: implement layer once the layer is done

                    ImGui::Separator(); // read only value
                    ImGui::Text("Display Value:");
                    Vector3D vel = rigidBody->getVelocity();
                    float velocity[3] = { vel.x, vel.y, vel.z };
                    ImGui::InputFloat3("Velocity", velocity, "%.3f", ImGuiInputTextFlags_ReadOnly);


                    Vector3D accel = rigidBody->getAcceleration();
                    float acceleration[3] = { accel.x, accel.y, accel.z };
                    ImGui::InputFloat3("Acceleration", acceleration, "%.3f", ImGuiInputTextFlags_ReadOnly);

                    Vector3D angVel = rigidBody->getAngularVelocity();
                    float angVelocity[3] = { angVel.x, angVel.y, angVel.z };
                    ImGui::InputFloat3("Angular Velocity", angVelocity, "%.3f", ImGuiInputTextFlags_ReadOnly);

                }

            }
        }
        else if constexpr (std::is_same_v<componentType, Collider>) {
            // rigidBody 
            if (Collider* collider = ImguiEcsRef.getComponent<Collider>(selectedEntityID)) {

                if (collider)
                {
                    ImGui::Separator();

                    Vector3D aabbHE = collider->getAABBHalfExtents();
                    float aabbHalfExtent[3] = { aabbHE.x, aabbHE.y, aabbHE.z };
                    if (ImGui::DragFloat3("AABB Half Extent", aabbHalfExtent, 0.1f)) {
                        collider->setAABBHalfExtents(Vector3D(aabbHalfExtent[0], aabbHalfExtent[1], aabbHalfExtent[2]));
                    }

                    Vector3D aabbOffset = collider->getAABBOffset();
                    float aabbOffsetData[3] = { aabbOffset.x, aabbOffset.y, aabbOffset.z };
                    if (ImGui::DragFloat3("AABB Offset", aabbOffsetData, 0.1f)) {
                        collider->setAABBHalfExtents(Vector3D(aabbOffsetData[0], aabbOffsetData[1], aabbOffsetData[2]));
                    }
                }

            }
        }
        else if constexpr (std::is_same_v<componentType, AudioComponent>) {
            // rigidBody 
            if (AudioComponent* audio = ImguiEcsRef.getComponent<AudioComponent>(selectedEntityID)) {

                if (audio)
                {
                    ImGui::Separator();
                    ImGui::Text("Audio Type:");
                    AudioType type = audio->getType();
                    int currentType = (type == AudioType::BGM ? 0 : 1);

                    if (ImGui::RadioButton("BGM", currentType == 0)) {
                        audio->setType(AudioType::BGM);
                    }
                    if (ImGui::RadioButton("SFX", currentType == 1)) {
                        audio->setType(AudioType::SFX);
                    }
                }

            }
        }
        else if constexpr (std::is_same_v<componentType, MeshComponent>) {
            // rigidBody 
            if (MeshComponent* mesh = ImguiEcsRef.getComponent<MeshComponent>(selectedEntityID)) {

                if (mesh)
                {
                    ImGui::Separator();

                   
                }

            }
            }



    }

}// end of namespace gam300