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
#include "../Manager/TracyManager.h"


#include "../Utility/AssetPath.h"

#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include "../Component/RenderComponent.h"
#include "../Component/Collider.h"
#include "../Component/AudioComponent.h"
#include "../Component/MeshComponent.h"
#include "../Component/Script.h"

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

    //static bool asset_editor = false;
    static bool prefab_editor = false;
    //static std::filesystem::directory_entry currentAsset;
    static std::filesystem::directory_entry selectedPrefab;

    static int tex_index = -1;

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

    ImguiManager::ImguiManager() : ImguiEcsRef(EM), ImguiGraphicRef(GFXM), m_descriptorEditor(nullptr) {}

    ImguiManager::ImguiManager(ECSManager& ECS, GraphicsManager& GFM) : ImguiEcsRef(ECS), ImguiGraphicRef(GFM), m_descriptorEditor(nullptr) {
        saveAsDefaultName[0] = '\0';
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

        // Initialize descriptor editor
        m_descriptorEditor = new DescriptorEditor(
            AM.db(),           // Get AssetDatabase from AssetManager
            AM.descriptorGenerator() // Get AssetDescriptorGenerator
        );

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

    void ImguiManager::displayFileList() {

        // ================ When Open File ===========================
        // scenePath: /Survival_Kit/Assets/Scene
        std::string scenePath = getAssetFilePath("Scene");
        std::vector<std::pair<std::string, std::string>> sceneFiles;

        if (std::filesystem::exists(scenePath) && std::filesystem::is_directory(scenePath))
        {
            for (const auto& file : std::filesystem::directory_iterator(scenePath)) {
                if (std::filesystem::is_regular_file(file.path())) {
                    std::string filename = file.path().filename().string();

                    ///filepath: Survival_Kit
                    // std::string filepath = file.path().string();
                    // std::cout << "Found scene file: " << filepath << std::endl;
                    //std::cout << "Scene Path: " << scenePath << std::endl;

                    sceneFiles.push_back(std::make_pair(file.path().filename().string(), file.path().string()));
                }
            }
        }

#if 1 // new code

        // ==================== True If File Open ==================
        if (fileWindow)
        {
            ImGui::OpenPopup("Level Select");
            //std::cout << "is open here\n";

        }

        if (ImGui::BeginPopupModal("Level Select", nullptr, ImGuiWindowFlags_NoDocking))
        {
            //std::cout << "is open here\n";
            ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);


            // =================== Loop through the level select menu =======================
            // sceneFiles[i]: ...../Scene\Game.scn, .... list of the file
            for (int i = 0; i < sceneFiles.size(); i++)
            {
                // fileName: Game.scn, GameB.scn .... List of filename for scene
                std::string fileName = sceneFiles[i].first;

                if (ImGui::Selectable(fileName.c_str())) {

                    ImguiEcsRef.clearAllEntities();

                    if (SEM.loadScene(sceneFiles[i].second))
                    {

                        shownFile = sceneFiles[i].second;
                        LM.writeLog("IMGUI_Manager::displayFileList(): Scene %s loaded successfully.", sceneFiles[i].first.c_str());

                    }
                    else {

                        LM.writeLog("IMGUI_Manager::displayFileList(): Scene %s failed to load. Loading default scene.", sceneFiles[i].first.c_str());


                        SEM.saveScene(getAssetFilePath("Scene/Game.scn"));

                        if (SEM.loadScene(getAssetFilePath("Scene/Game.scn"))) {

                            LM.writeLog("IMGUI_Manager::displayFileList(): Default scene loaded successfully.");

                        }
                        else {

                            LM.writeLog("IMGUI_Manager::displayFileList(): WARNING: Failed to load default scene.");

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

                    if (!ImguiEcsRef.hasAllComponents<MeshComponent, Transform3D>(createNewEntity.get_id())) {
                        ImguiEcsRef.addComponent<MeshComponent>(createNewEntity.get_id());
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
                        //========================Delete Entity=====================================
                        if (ImGui::MenuItem("Delete"))
                        {
                            if (selectedObjIndex >= 0 && selectedObjIndex < static_cast<int>(allEntities.size()))
                            {
                                EntityID idToDelete = allEntities[selectedObjIndex].get_id();
                                ImguiEcsRef.destroyEntity(idToDelete);

                                selectedObjIndex = -1;
                            }
                        }
                        //================= Duplicate Function =============================
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
                                if (ImguiEcsRef.hasComponent<Script>(oriSelectedEntity.get_id()))
                                {
                                    Script* script = ImguiEcsRef.getComponent<Script>(oriSelectedEntity.get_id());
                                    if (script)
                                    {
                                        ImguiEcsRef.addComponent<Script>(newEntityID, *script);
                                    }
                                }
                                selectedObjIndex = static_cast<int>(allEntities.size()) - 1;

                            }
                        }
                        ImGui::Separator();
                        // ========================= Prefabs function ====================================
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

                //========================== Display Added Component ========================================
                // Display adjustable value in components  
                if (ImguiEcsRef.hasComponent<Transform3D>(selectedEntity.get_id())) {
                    displayComponentMenu<Transform3D>(selectedEntity.get_id(), "Transform3D");

                }
                if (ImguiEcsRef.hasComponent<RigidBody>(selectedEntity.get_id())) {
                    displayComponentMenu<RigidBody>(selectedEntity.get_id(), "RigidBody");


                }
                if (ImguiEcsRef.hasComponent<Collider>(selectedEntity.get_id())) {
                    displayComponentMenu<Collider>(selectedEntity.get_id(), "Collider");


                }
                if (ImguiEcsRef.hasComponent<AudioComponent>(selectedEntity.get_id())) {
                    displayComponentMenu<AudioComponent>(selectedEntity.get_id(), "Audio");

                }
                if (ImguiEcsRef.hasComponent<MeshComponent>(selectedEntity.get_id())) {
                    displayComponentMenu<MeshComponent>(selectedEntity.get_id(), "Mesh");


                }
                if (ImguiEcsRef.hasComponent<Script>(selectedEntity.get_id())) {
                    displayComponentMenu<Script>(selectedEntity.get_id(), "Script");


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
                // =====================Add Component ====================================
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
                    if (ImGui::MenuItem("Script"))
                    {
                        if (!ImguiEcsRef.hasComponent<Script>(selectedEntity.get_id()))
                        {
                            ImguiEcsRef.addComponent<Script>(selectedEntity.get_id());
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

    //void ImguiManager::showPrefabsPanel(EntityID selectedEntity)
    //{
    //    //if (ImGui::Begin)
    //}

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
                    // shownFile: Survival_Kit/Assets/Scene\Game.scn
                    SEM.saveScene(shownFile);
                    //std::cout << "test here" << shownFile << "\n";

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
            size_t pos = shownFile.find("Assets");
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

        // ========================== Open top menu =============================  
        if (fileWindow) {

            IMGUIM.displayFileList();
        }

        // newPath: /Survival_Kit/Assets/Scene/
        std::string newPath = getAssetFilePath("Scene/") + saveAsDefaultName;

        // ======================= Save as Top Menu Panel ============================
        if (showSaveAsPanel)
        {
            ImGui::OpenPopup("Save Scene As");

            if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::InputText("File name", saveAsDefaultName, IM_ARRAYSIZE(saveAsDefaultName));

                if (ImGui::Button("Save", ImVec2(120, 0)))
                {
                    // newPath: /Survival_Kit/Assets/Scene/Test4.scn
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

                        // shownFile: /Survival_Kit/Assets/Scene/NewTest3.scn
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

                // =================== Top Menu Pop up overwrite ==================== 
                if (ImGui::BeginPopupModal("Confirm Overwrite", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("File %s already exists.\nDo you want to replace it?", saveAsDefaultName);
                    ImGui::Separator();

                    if (ImGui::Button("Yes", ImVec2(120, 0)))
                    {

                        // newPath: / Survival_Kit / Assets / Scene / Game.scn
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
        delete m_descriptorEditor;
        m_descriptorEditor = nullptr;
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    Vector2D ImguiManager::getWindowSize(GLFWwindow& window)
    {
        //Vector2D dimension{ 0,0 };

        glfwGetWindowSize(&window, &width, &height);
        return Vector2D(static_cast<float>(width), static_cast<float>(height));
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

            handleViewPortClick(imagePos);
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
        if (std::filesystem::exists(getAssetsPath()) && std::filesystem::is_directory(getAssetsPath()))
        {
            for (const auto& entry : std::filesystem::directory_iterator(getAssetsPath()))
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
        //static int selectedAssetIndex = -1;
        //std::cout << "selectedFolder: " << selectedFolder << "\n";
        // ---set up 2 column ---
        ImGui::Columns(2, nullptr, true);

        // ----- Left column panel:  ----- //
        ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Projects:");
        if (ImGui::CollapsingHeader("Assets", ImGuiTreeNodeFlags_DefaultOpen))
        {

            // folder: Textures, Audio, Descriptors
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
                for (const auto& folderEntry : std::filesystem::directory_iterator(getAssetsPath()))
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
                //std::cout << "\nselected Folder: " << selectedFolder << "\n";
                // normal view: only selected folder
                std::filesystem::path currentFolder = std::filesystem::path(getAssetsPath()) / selectedFolder;
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
            int itemsPerRow = (int)(panelWidth / cellSize);
            if (itemsPerRow < 1)
            {
                itemsPerRow = 1;

            }

            int texture_count = -1;
            ImGui::Columns(itemsPerRow, nullptr, false);
            for (size_t i = 0; i < assetsList.size(); ++i)
            {
                const auto& assetEntry = assetsList[i];
                std::string filename = assetEntry.path().filename().string();
                std::string fileNamePath = assetEntry.path().string();

                if (assetEntry.path().extension().string() == ".png" || assetEntry.path().extension().string() == ".jpeg") //to open the image
                {
                    ++texture_count;
                }
                ImGui::PushID(filename.c_str());
                if (ImGui::Button(filename.c_str(), ImVec2(thumbnailSize, thumbnailSize))) {

                    selectedAssetIndex = static_cast<int>(i);
                    if (m_descriptorEditor->LoadDescriptor(AM.getAssetIdByFilename(filename), m_currentDescriptor)) {
                        m_showDescriptorPanel = true;
                        LM.writeLog("Loaded descriptor for asset: %s", filename.c_str());
                    }
                    else {
                        LM.writeLog("Failed to load descriptor for asset ID: %llu", AM.getAssetIdByFilename(filename));
                    }

                    selectedAssetIndex = static_cast<int>(i);
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
                        tex_index = texture_count;

                    }
                    else {
                        tex_index = -1;
                    }
                    if (extension == ".prefab") //to open the image
                    {
                        prefab_editor = true;
                        selectedPrefab = assetEntry;
                        //m_showDescriptorPanel = true;
                    }
                }

                // ===============show toolip detail ======================
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

            /*if (asset_editor) {
                displayAssetEditor(currentAsset);
            }*/

            if (prefab_editor) {
                displayPrefabEditor(selectedPrefab);
            }
        }

        ImGui::EndChild();
        ImGui::Columns(1);
        ImGui::End();
    }


#endif

    void ImguiManager::displayAssetEditor()
    {

        if (!m_showDescriptorPanel) return;

        if (ImGui::Begin("Descriptor Editor", &m_showDescriptorPanel)) {

            ImGui::Columns(2, nullptr, true);
            auto& texture_store = GFXM.getTextureStorage();

            if (tex_index != -1) {
                // Get texture size
                float tex_w = static_cast<float>(texture_store[tex_index].width());
                float tex_h = static_cast<float>(texture_store[tex_index].height());

                // Get window size
                float win_w = static_cast<float>(getWindowWidthHeight().x);
                float win_h = static_cast<float>(getWindowWidthHeight().y) / 2.0f;

                // Compute texture aspect ratio
                float aspect = tex_w / tex_h;

                // Compute viewport size preserving aspect ratio
                ImVec2 viewportSize;
                if (win_w / win_h > aspect) {
                    viewportSize.x = win_h * aspect;
                    viewportSize.y = win_h;
                }
                else {
                    viewportSize.x = win_w;
                    viewportSize.y = win_w / aspect;
                }

                // Draw the image
                ImGui::Image(
                    (ImTextureID)(intptr_t)((GLuint)texture_store[tex_index].handle()),
                    viewportSize,
                    ImVec2(0, 1), ImVec2(1, 0)
                );

            }

            ImGui::NextColumn();

            // Display read-only asset info
            ImGui::SeparatorText("Asset Information");
            ImGui::Text("Asset ID: %llu", m_currentDescriptor.assetId);
            ImGui::Text("GUID: %s", m_currentDescriptor.guid.c_str());
            ImGui::Text("Source: %s", m_currentDescriptor.sourcePath.c_str());
            ImGui::Text("Type: %s", getAssetTypeName(m_currentDescriptor.assetType));

            ImGui::Spacing();
            ImGui::SeparatorText("Editable Properties");

            // Display Name (editable)
            char displayNameBuf[256];
            strncpy_s(displayNameBuf, sizeof(displayNameBuf),
                m_currentDescriptor.displayName.c_str(), _TRUNCATE);
            if (ImGui::InputText("Display Name", displayNameBuf, sizeof(displayNameBuf))) {
                m_descriptorEditor->UpdateProperty(
                    m_currentDescriptor.assetId,
                    "displayName",
                    std::string(displayNameBuf)
                );
                m_currentDescriptor.displayName = displayNameBuf;
            }

            // Category (editable)
            char categoryBuf[128];
            strncpy_s(categoryBuf, sizeof(categoryBuf),
                m_currentDescriptor.category.c_str(), _TRUNCATE);
            if (ImGui::InputText("Category", categoryBuf, sizeof(categoryBuf))) {
                m_descriptorEditor->UpdateProperty(
                    m_currentDescriptor.assetId,
                    "category",
                    std::string(categoryBuf)
                );
                m_currentDescriptor.category = categoryBuf;
            }

            // Texture-specific settings (only for textures)
            if (m_currentDescriptor.assetType == AssetType::Texture) {
                ImGui::Spacing();
                ImGui::SeparatorText("Texture Settings");

                // Usage Type dropdown
                std::vector<std::string> usageTypes = m_descriptorEditor->GetPropertyOptions("textureSettings.usageType");
                if (ImGui::BeginCombo("Usage Type", m_currentDescriptor.textureSettings.usageType.c_str())) {
                    for (const auto& type : usageTypes) {
                        bool isSelected = (m_currentDescriptor.textureSettings.usageType == type);
                        if (ImGui::Selectable(type.c_str(), isSelected)) {
                            m_descriptorEditor->UpdateProperty(
                                m_currentDescriptor.assetId,
                                "textureSettings.usageType",
                                type
                            );
                            m_currentDescriptor.textureSettings.usageType = type;
                        }
                    }
                    ImGui::EndCombo();
                }

                // Compression dropdown
                std::vector<std::string> compressionTypes = m_descriptorEditor->GetPropertyOptions("textureSettings.compression");
                if (ImGui::BeginCombo("Compression", m_currentDescriptor.textureSettings.compression.c_str())) {
                    for (const auto& comp : compressionTypes) {
                        bool isSelected = (m_currentDescriptor.textureSettings.compression == comp);
                        if (ImGui::Selectable(comp.c_str(), isSelected)) {
                            m_descriptorEditor->UpdateProperty(
                                m_currentDescriptor.assetId,
                                "textureSettings.compression",
                                comp
                            );
                            m_currentDescriptor.textureSettings.compression = comp;
                        }
                    }
                    ImGui::EndCombo();
                }

                // Quality slider
                float quality = m_currentDescriptor.textureSettings.quality;
                if (ImGui::SliderFloat("Quality", &quality, 0.0f, 1.0f)) {
                    m_descriptorEditor->UpdateProperty(
                        m_currentDescriptor.assetId,
                        "textureSettings.quality",
                        std::to_string(quality)
                    );
                    m_currentDescriptor.textureSettings.quality = quality;
                }

                // Generate Mipmaps checkbox
                bool genMips = m_currentDescriptor.textureSettings.generateMipmaps;
                if (ImGui::Checkbox("Generate Mipmaps", &genMips)) {
                    m_descriptorEditor->UpdateProperty(
                        m_currentDescriptor.assetId,
                        "textureSettings.generateMipmaps",
                        genMips ? "true" : "false"
                    );
                    m_currentDescriptor.textureSettings.generateMipmaps = genMips;
                }

                // sRGB checkbox
                bool srgb = m_currentDescriptor.textureSettings.srgb;
                if (ImGui::Checkbox("sRGB", &srgb)) {
                    m_descriptorEditor->UpdateProperty(
                        m_currentDescriptor.assetId,
                        "textureSettings.srgb",
                        srgb ? "true" : "false"
                    );
                    m_currentDescriptor.textureSettings.srgb = srgb;
                }
            }

            // Dirty state indicator
            if (m_currentDescriptor.isDirty) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Modified");
            }

            ImGui::Columns(1);
        }

        ImGui::End();
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

    void ImguiManager::handleViewPortClick(ImVec2 mousePos)
    {
        // Check the mouse hover position for reference
        if (ImGui::IsItemHovered())
        {
            ImVec2 mouseScreen = ImGui::GetIO().MousePos;

            ImVec2 mouseInViewportPos;
            mouseInViewportPos.x = mouseScreen.x - mousePos.x;
            mouseInViewportPos.y = mouseScreen.y - mousePos.y;

            ImGui::Text("Mouse local: (%.1f, %.1f)", mouseInViewportPos.x, mouseInViewportPos.y);


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

                    //static bool forceMask = true;
                    //if (ImGui::Checkbox("ForceMask", &forceMask)) {
                    //    rigidBody->setForceMask(forceMask ? 0xFFFFFFFFu : 0u);
                    //}

                    //static bool torqueMask = true;
                    //if (ImGui::Checkbox("Torque Mask", &torqueMask)) {
                    //    rigidBody->setTorqueMask(torqueMask ? 0xFFFFFFFFu : 0u);
                    //}

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
            // audio
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

                    ImGui::Separator();
                    ImGui::Text("Play State:");
                    PlayState playState = audio->getPlayState();
                    if (ImGui::RadioButton("Play", playState == PlayState::PLAY)) {
                        audio->setPlayState(PlayState::PLAY);
                    }
                    if (ImGui::RadioButton("Pause", playState == PlayState::PAUSE)) {
                        audio->setPlayState(PlayState::PAUSE);
                    }
                    if (ImGui::RadioButton("Stop", playState == PlayState::STOP)) {
                        audio->setPlayState(PlayState::STOP);
                    }

                    ImGui::Separator();
                    ImGui::Text("Volume:");
                    float volume = audio->getVolume();
                    if (ImGui::DragFloat("Volume", &volume, 0.001f, 0.f, 1.f)) {
                        audio->setVolume(volume);
                    }

                    ImGui::Separator();
                    ImGui::Text("Pitch:");
                    float pitch = audio->getPitch();
                    if (ImGui::DragFloat("Pitch", &pitch, 0.001f, 0.f, 1.f)) {
                        audio->setPitch(pitch);
                    }

                    ImGui::Separator();
                    ImGui::Text("Looping:");
                    bool looping = audio->isLooping();
                    if (ImGui::Checkbox("Looping", &looping)) {
                        audio->setLooping(looping);
                    }

                    ImGui::Separator();
                    ImGui::Text("Mute:");
                    bool mute = audio->isMute();
                    if (ImGui::Checkbox("Mute", &mute)) {
                        audio->setMute(mute);
                    }

                    ImGui::Separator();
                    ImGui::Text("3D:");
                    bool is_3d = audio->is3D();
                    if (ImGui::Checkbox("3D", &is_3d)) {
                        audio->setIs3D(is_3d);
                    }

                    ImGui::Separator();

                    std::string advice = "Max Distance needs to be higher than Min Distance to have attenuation";
                    ImGui::TextDisabled("(i)");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted(advice.c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }

                    // Disable only if not 3D
                    ImGui::BeginDisabled(!is_3d);

                    ImGui::Text("Min Distance:");
                    float min_distance = audio->getMinDistance();
                    if (ImGui::DragFloat("##MinDistance", &min_distance, 0.1f, 0.f)) {
                        if (is_3d) {
                            audio->setMinDistance(min_distance);
                        }
                        else {
                            audio->setMinDistance(1.f);
                        }
                    }

                    ImGui::Separator();

                    ImGui::Text("Max Distance:");
                    float max_distance = audio->getMaxDistance();
                    if (ImGui::DragFloat("##MaxDistance", &max_distance, 0.1f, 0.f)) {
                        if (is_3d) {
                            audio->setMaxDistance(max_distance);
                        }
                        else {
                            audio->setMaxDistance(10.f);
                        }
                    }

                    ImGui::EndDisabled();

                }

            }
        }
        else if constexpr (std::is_same_v<componentType, MeshComponent>) {
            // rigidBody 
            if (MeshComponent* mesh = ImguiEcsRef.getComponent<MeshComponent>(selectedEntityID)) {

                ImGui::Separator();
                //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                ImGui::Text("GUID: %s", mesh->getGUID().c_str());
                ImGui::Separator();

                // =============== Mesh Dropdown =======================

                uint16_t currMesh = mesh->getMeshHandle();
                std::string meshName = ImguiGraphicRef.getMeshName(currMesh);

                if (ImGui::BeginCombo("Mesh Shape Type", meshName.c_str())) {

                    size_t meshCount = ImguiGraphicRef.getMeshCount();

                    for (uint16_t i = 0; i < meshCount; i++)
                    {
                        std::string name = ImguiGraphicRef.getMeshName(i);
                        bool selected = (currMesh == i);

                        if (ImGui::Selectable(name.c_str(), selected)) {
                            mesh->setMeshHandle(i);
                            mesh->setGUID(ImguiGraphicRef.getMeshGUID(i));
                        }

                        if (selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Spacing();

                // =============== Meterial Dropdown =======================
                uint16_t currMaterial = mesh->getMaterialHandle();
                //ImGui::Text("Debug - Current Material Handle: %d", currMaterial);
                std::string materialName;
                if (currMaterial == 0) {
                    materialName = "None";
                }
                else {
                    materialName = "Material " + std::to_string(currMaterial);
                }
                if (ImGui::BeginCombo("Material", materialName.c_str())) {
                    bool noneSelected = (currMaterial == 0);
                    if (ImGui::Selectable("None", noneSelected))
                    {
                        mesh->setMaterialHandle(0);

                    }
                    if (noneSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                    ImGui::Separator();

                    // ================ Show materials ==========================
                    const auto& materials = ImguiGraphicRef.getMaterialStorage();
                    if (materials.empty())
                    {
                        ImGui::TextDisabled("(No materials loaded)");
                    }
                    else
                    {
                        for (uint16_t handle = 0; handle < materials.size(); ++handle) {
                            std::string matName = "Material " + std::to_string(handle);
                            bool selected = (currMaterial == handle);
                            if (ImGui::Selectable(matName.c_str(), selected)) {
                                mesh->setMaterialHandle(handle);

                            }
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                    }
                    ImGui::EndCombo();

                }

            }
        }
        else if constexpr (std::is_same_v<componentType, Script>) {
            // script 
            if (Script* script = ImguiEcsRef.getComponent<Script>(selectedEntityID)) {

                ImGui::Separator();

                char scriptNameBuffer[128];

                strcpy_s(scriptNameBuffer, sizeof(scriptNameBuffer), script->getScriptName().c_str());

                if (ImGui::InputText("Script Name", scriptNameBuffer, sizeof(scriptNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    std::string newScriptName = scriptNameBuffer;

                    if (newScriptName.empty()) {
                        newScriptName = script->getScriptName();
                    }

                    script->setScriptName(newScriptName);
                }

                ImGui::Separator();

                bool active = script->isActive();
                if (ImGui::Checkbox("Active", &active)) {
                    script->setActive(active);
                }



            }
        }
    }

    void ImguiManager::displayPerformanceProfile()
    {
        ImGui::SetNextWindowSize(ImVec2(200, 100));

        if (ImGui::Begin("Performance Profile", &hierachyWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            if (ImGui::Button("Launch Tracy Window"))
            {
                if (!TRACY.isRunning())
                {
                    TRACY.launchTracy();

                }
            }

            TRACY.update();
            ImGui::Separator();

            // Use FPS calculated in Main.cpp for consistency
            float currentFPS = (lastReceivedFPS > 0.0f) ? lastReceivedFPS : ImGui::GetIO().Framerate;
            float currentFrameTime = 1000.0f / currentFPS; //converting to milliseconds

            //history statistics
            fpsHistory[fpsHistoryOffset] = currentFPS;
            frameTimeHistory[fpsHistoryOffset] = currentFrameTime;
            fpsHistoryOffset = (fpsHistoryOffset + 1) % FPS_HISTORY_SIZE;

            //update min/max statistics
            minFPS = (currentFPS < minFPS) ? currentFPS : minFPS;
            maxFPS = (currentFPS > maxFPS) ? currentFPS : maxFPS;
            minFrameTime = (currentFrameTime < minFrameTime) ? currentFrameTime : minFrameTime;
            maxFrameTime = (currentFrameTime > maxFrameTime) ? currentFrameTime : maxFrameTime;

            //calculate average
            float avgFPS = 0.0f;
            float avgFrameTime = 0.0f;
            for (int i = 0; i < FPS_HISTORY_SIZE; i++)
            {
                avgFPS += fpsHistory[i];
                avgFrameTime += frameTimeHistory[i];
            }
            avgFPS /= (float)FPS_HISTORY_SIZE;
            avgFrameTime /= (float)FPS_HISTORY_SIZE;

            //showcase statistics
            ImGui::Text("Frame Statistics");
            ImGui::Spacing();

            //create a table to display statistics better
            if (ImGui::BeginTable("StatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableHeadersRow();

                // Average FPS
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Average FPS:");
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", avgFPS);
                ImGui::TableNextColumn();
                ImGui::Text("fps");

                // Average Frame Time
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Avg Frame Time:");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", avgFrameTime);
                ImGui::TableNextColumn();
                ImGui::Text("ms");

                // Min Frame Time
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Min Frame Time:");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", minFrameTime);
                ImGui::TableNextColumn();
                ImGui::Text("ms");

                // Max Frame Time
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Max Frame Time:");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", maxFrameTime);
                ImGui::TableNextColumn();
                ImGui::Text("ms");

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            //showcase performance graphs section
            ImGui::Text("Performance Graphs");
            ImGui::Spacing();

            float graphWidth = ImGui::GetContentRegionAvail().x;

            //FPS graph
            char fpsOverlay[64];
            sprintf_s(fpsOverlay, sizeof(fpsOverlay), "FPS - avg %.1f", avgFPS);

            float fpsMinScale = (avgFPS - 30.0f > 0.0f) ? (avgFPS - 30.0f) : 0.0f;
            float fpsMaxScale = avgFPS + 30.0f;

            ImGui::PlotLines(
                "##FPS",
                fpsHistory,
                FPS_HISTORY_SIZE,
                fpsHistoryOffset,
                fpsOverlay,
                fpsMinScale,
                fpsMaxScale,
                ImVec2(graphWidth, 100.0f),
                sizeof(float)
            );

            // frame time graph
            char frameTimeOverlay[64];
            sprintf_s(frameTimeOverlay, sizeof(frameTimeOverlay), "Frame Time (ms) - avg %.2f", avgFrameTime);

            //dynamic scaling
            float ftMinScale = (avgFrameTime - 5.0f > 0.0f) ? (avgFrameTime - 5.0f) : 0.0f;
            float ftMaxScale = avgFrameTime + 5.0f;

            ImGui::PlotLines(
                "##FrameTime",
                frameTimeHistory,
                FPS_HISTORY_SIZE,
                fpsHistoryOffset,
                frameTimeOverlay,
                ftMinScale,
                ftMaxScale,
                ImVec2(graphWidth, 100.0f),
                sizeof(float)
            );

            ImGui::Spacing();
            ImGui::Separator();

            // different coloring to indicate performance status
            ImGui::Spacing();
            if (currentFPS >= 60.0f)
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance: Excellent");
            else if (currentFPS >= 30.0f)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance: Good");
            else
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Performance: Poor");

            ImGui::Spacing();



        }

        ImGui::End();
    }

    void ImguiManager::updateFPS(float fps)
    {
        lastReceivedFPS = fps;
        fpsHistory[fpsHistoryOffset] = fps;
        fpsHistoryOffset = (fpsHistoryOffset + 1) % FPS_HISTORY_SIZE;
    }

}// end of namespace gam300