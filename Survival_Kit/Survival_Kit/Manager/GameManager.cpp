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
#include "PrefabManager.h"
#include "GraphicsManager.h"

// Include component headers
#include "../Component/Transform3D.h"
#include "../Component/AudioComponent.h"
#include "../Component/RigidBody.h"
#include "../Component/Bullet.h"
#include "../Component/Script.h"
#include "../Component/MeshComponent.h"
#include "../Component/TextureComponent.h"

// Include system headers
#include "../System/MovementSystem.h"
#include "../System/PhysicsSystem.h"
#include "../System/RenderSystem.h"
#include "../Component/RigidBody.h"
#include "../System/BulletSystem.h"
#include "../System/CollisionSystem.h"
#include "../System/TransformSystem.h"
#include "../System/AudioSystem.h"

// Include Utility headers
#include "../Utility/Clock.h"
#include "../Utility/AssetPath.h"

// Include Tracy headers
#include "../Tracy/tracy/Tracy.hpp"

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
        if (LM.startUp()) {
            // Failed to start logManager
            return -1;
        }

        LM.writeLog("GameManager::startUp() - logManager started successfully");

        // Start the InputManager
        if (IM.startUp()) {
            LM.writeLog("GameManager::startUp() - Failed to start InputManager");
            LM.shutDown();
            return -1;
        }

        LM.writeLog("GameManager::startUp() - InputManager started successfully");

        // Start the ECSManager
        if (EM.startUp()) {
            LM.writeLog("GameManager::startUp() - Failed to start ECSManager");
            IM.shutDown();
            LM.shutDown();
            return -1;
        }

        LM.writeLog("GameManager::startUp() - ECSManager started successfully");

        // Start the SerialisationManager
        if (SEM.startUp()) {
            LM.writeLog("GameManager::startUp() - Failed to start SerialisationManager");
            EM.shutDown();
            IM.shutDown();
            LM.shutDown();
            return -1;
        }

        LM.writeLog("GameManager::startUp() - SerialisationManager started successfully");

        // Start the PrefabManager
        if (PM.startUp()) {
            LM.writeLog("GameManager::startUp() - Failed to start PrefabManager");
            EM.shutDown();
            IM.shutDown();
            LM.shutDown();
            return -1;
        }

        LM.writeLog("GameManager::startUp() - PrefabManager started successfully");

        // Start the GraphicsManager
        if (GFXM.startUp()) {
            LM.writeLog("GameManager::startUp() - Failed to start GraphicsManager");
            EM.shutDown();
            IM.shutDown();
            SEM.shutDown();
            LM.shutDown();
            return -1;
        }

        LM.writeLog("GameManager::startUp() - GraphicsManager started successfully");

		// Register components
        CM.register_component<Transform3D>();
        LM.writeLog("GameManager::startUp() - Transform3D component registered successfully");

        CM.register_component<RigidBody>();
        LM.writeLog("GameManager::startUp() - RigidBody component started successfully");

        CM.register_component<Collider>();
        LM.writeLog("GameManager::startUp() - Collider component started successfully");

		CM.register_component<AudioComponent>();
		logManager.writeLog("GameManager::startUp() - AudioComponent component registered successfully");

        CM.register_component<RenderComponent>();
        logManager.writeLog("GameManager::startUp() - RenderComponent component registered successfully");

		CM.register_component<Bullet>();
		LM.writeLog("GameManager::startUp() - Bullet component registered successfully");

		CM.register_component<Script>();
		LM.writeLog("GameManager::startUp() - Script component registered successfully");

        CM.register_component<MeshComponent>();
        LM.writeLog("GameManager::startUp() - Mesh component registered successfully");

        CM.register_component<TextureComponent>();
        LM.writeLog("GameManager::startUp() - Texture componennt registered successfully");

        // Check the scene path
        const std::string scenePath = getAssetFilePath("Scene/Game.scn");
        LM.writeLog("GameManager::startUp() - Attempting to load scene from '%s'", scenePath.c_str());



		// Register the Systems with the ECSManager
        SM.register_system<MovementSystem>();
        SM.register_system<PhysicsSystem>();
        SM.register_system<CollisionSystem>();
		SM.register_system<BulletSystem>();
        SM.register_system<RenderSystem>();
        SM.register_system<TransformSystem>();
        SM.register_system<AudioSystem>();

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
        GFXM.shutDown();
		PM.shutDown();
        SEM.shutDown();
        EM.shutDown();
        IM.shutDown();
        logManager.shutDown();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Update the game state for the current frame
    void GameManager::update(float dt) {

        ZoneScoped;

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