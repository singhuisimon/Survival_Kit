/**
 * @file GameManager.cpp
 * @brief Implementation of the Game Manager for the game engine.
 * @details Manages the game state, frame timing, and overall game systems.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

 // Include managers and utility headers
#include "GameManager.h"
#include "LogManager.h"
#include "InputManager.h" 
#include "ECSManager.h"
#include "SerialisationManager.h"
#include "GraphicsManager.h"
#include "../Component/Transform3D.h"
#include "../Utility/Clock.h"
#include "../Utility/AssetPath.h"
#include "../System/MovementSystem.h"
#include "../Component/RigidBody.h"

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

        // Start the InputManager
        if (IM.startUp()) {
            logManager.writeLog("GameManager::startUp() - Failed to start InputManager");
            logManager.shutDown();
            return -1;
        }

        logManager.writeLog("GameManager::startUp() - InputManager started successfully");

        // Start the ECSManager
        if (EM.startUp()) {
            logManager.writeLog("GameManager::startUp() - Failed to start ECSManager");
            IM.shutDown();
            logManager.shutDown();
            return -1;
        }

        logManager.writeLog("GameManager::startUp() - ECSManager started successfully");

        // Start the SerialisationManager
        if (SEM.startUp()) {
            logManager.writeLog("GameManager::startUp() - Failed to start SerialisationManager");
            EM.shutDown();
            IM.shutDown();
            logManager.shutDown();
            return -1;
        }

        logManager.writeLog("GameManager::startUp() - SerialisationManager started successfully");

        // Start the GraphicsManager
        if (GFXM.startUp()) {
            logManager.writeLog("GameManager::startUp() - Failed to start GraphicsManager");
            EM.shutDown();
            IM.shutDown();
            SEM.shutDown();
            logManager.shutDown();
            return -1;
        }

        logManager.writeLog("GameManager::startUp() - GraphicsManager started successfully");

        // Register the Transform3D component with the ComponentManager
        CM.register_component<Transform3D>();
        logManager.writeLog("GameManager::startUp() - Transform3D component registered successfully");

        // Register the RigidBody component with the componentManager
        CM.register_component<RigidBody>();
        logManager.writeLog("GameManager::startUp() - RigidBody component registered successfully");
        // Register RigidBody component with the componentManager
        CM.register_component<RigidBody>();
        logManager.writeLog("GameManager::startUp() - GraphicsManager started successfully");


        // Register the Movement component with the ComponetManager
        SM.register_system<MovementSystem>();

        //// Create a test entity with Transform3D component for demonstration
        //Entity& testEntity = EM.createEntity("TestEntity");
        //Vector3D position(0.0f, 0.0f, 0.0f);
        //Vector3D rotation(0.0f, 45.0f, 0.0f);  // 45 degrees rotation on Y axis
        //Vector3D scale(1.0f, 1.0f, 1.0f);

        //Transform3D* transform = EM.addComponent<Transform3D>(testEntity.get_id(), position, rotation, scale);
        //if (transform) {
        //    logManager.writeLog("GameManager::startUp() - Test entity created with Transform3D component");
        //}

        // Load the scene - Commented out to load scene using editor instead (Edited - Lily (15/9))
        //const std::string scenePath = getAssetFilePath("Scene/Game.scn");
        //if (SEM.loadScene(scenePath)) {
        //    logManager.writeLog("GameManager::startUp() - Scene loaded successfully from %s", scenePath.c_str());
        //}
        //else {
        //    logManager.writeLog("GameManager::startUp() - Failed to load scene, creating default scene");
        //    // Save to the same path
        //    SEM.saveScene(scenePath);
        //    if (SEM.loadScene(scenePath)) {
        //        logManager.writeLog("GameManager::startUp() - Default scene loaded successfully");
        //    }
        //    else {
        //        logManager.writeLog("GameManager::startUp() - WARNING: Failed to load default scene");
        //    }
        //}

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

        // Shut down managers in reverse order of initialization
        SEM.shutDown();
        EM.shutDown();
        IM.shutDown();
        logManager.shutDown();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Update the game state for the current frame
    void GameManager::update(float dt) {
        // Increment step count
        m_step_count++;

        // Log every 100 steps
        if (m_step_count % 100 == 0) {
            LM.writeLog("GameManager::update() - Step count: %d", m_step_count);
        }

        // Check for escape key to quit
        if (IM.isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            setGameOver(true);
            LM.writeLog("GameManager::update() - Escape key pressed, setting game over");
        }

        // Update all ECS systems
        EM.updateSystems(dt);

        // Example: Work with serialized entities using new lookup functionality
        workWithSerializedEntities(dt);
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

    // Work with serialized entities using new lookup functionality
    void GameManager::workWithSerializedEntities(float dt) {
        // Find the Cube entity that was loaded from the scene file
        Entity* cubeEntity = EM.getEntityByName("Cube");
        if (cubeEntity) {
            Transform3D* cubeTransform = EM.getComponent<Transform3D>(cubeEntity->get_id());
            if (cubeTransform) {
                // Example: Rotate the cube slowly
                Vector3D currentRotation = cubeTransform->getRotation();
                currentRotation.y += dt * 30.0f; // 30 degrees per second
                cubeTransform->setRotation(currentRotation);

                // Log position every 5 seconds for debugging
                static float logTimer = 0.0f;
                logTimer += dt;
                if (logTimer >= 5.0f) {
                    const Vector3D& pos = cubeTransform->getPosition();
                    LM.writeLog("GameManager::workWithSerializedEntities() - Cube position: (%.1f, %.1f, %.1f)",
                        pos.x, pos.y, pos.z);
                    logTimer = 0.0f;
                }
            }
        }
        else {
            // Log warning if cube entity not found (but only once to avoid spam)
            static bool warningLogged = false;
            if (!warningLogged) {
                LM.writeLog("GameManager::workWithSerializedEntities() - WARNING: Cube entity not found in scene");
                warningLogged = true;
            }
        }
    }

    // Helper method to clear entities before loading new scene
    void GameManager::loadNewScene(const std::string& scenePath) {
        LM.writeLog("GameManager::loadNewScene() - Loading new scene: %s", scenePath.c_str());

        // Clear existing entities before loading new scene
        EM.clearAllEntities();
        LM.writeLog("GameManager::loadNewScene() - Existing entities cleared");

        // Load the new scene
        if (SEM.loadScene(scenePath)) {
            LM.writeLog("GameManager::loadNewScene() - New scene loaded successfully");
        }
        else {
            LM.writeLog("GameManager::loadNewScene() - Failed to load new scene");
        }
    }

    // Helper method to save current game state
    void GameManager::saveCurrentGame(const std::string& saveSlot) {
        std::string savePath = getAssetFilePath("Saves/save_" + saveSlot + ".scn");
        LM.writeLog("GameManager::saveCurrentGame() - Saving game to slot '%s' at path '%s'",
            saveSlot.c_str(), savePath.c_str());

        // Save current scene (entities remain in memory for continued gameplay)
        if (SEM.saveScene(savePath)) {
            LM.writeLog("GameManager::saveCurrentGame() - Game saved successfully");
        }
        else {
            LM.writeLog("GameManager::saveCurrentGame() - Failed to save game");
        }
    }

} // end of namespace gam300