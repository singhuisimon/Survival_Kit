/**
 * @file ScriptSystem.cpp
 * @brief Implementation of the ScriptSystem for ECS script management.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Manages script execution and lifecycle in the ECS, including
 *          initialization, per-entity updates, hot-reload monitoring, and
 *          script-to-entity binding through the C# scripting engine.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ScriptSystem.h"
#include "../Manager/ECSManager.h"
#include "../Manager/LogManager.h"
#include "../ScriptCore/Application.h"

namespace gam300 {

    ScriptSystem::ScriptSystem()
        : System("ScriptSystem")
        , m_script_app(nullptr)
        , m_script_engine_initialized(false) {
    }

    ScriptSystem::~ScriptSystem() {
        shutdown();
    }

    bool ScriptSystem::init(SystemManager& system_manager) {
        LM.writeLog("ScriptSystem::init() - Initializing Script System");

        // Create the script engine application
        m_script_app = new Core::Application();

        try {
            // Initialize the scripting engine
            m_script_app->InitializeScripting();
            m_script_engine_initialized = true;

            LM.writeLog("ScriptSystem::init() - Script engine initialized successfully");
            return true;
        }
        catch (const std::exception& e) {
            LM.writeLog("ScriptSystem::init() - Failed to initialize script engine: %s", e.what());
            delete m_script_app;
            m_script_app = nullptr;
            m_script_engine_initialized = false;
            return false;
        }
    }

    void ScriptSystem::update(float dt) {
        if (!m_script_engine_initialized || !m_script_app) {
            return;
        }

        // Check for script file changes and auto-reload if needed (once per frame)
        m_script_app->CheckAndReloadScripts();

        // Update only entities with active Script components
        for (EntityID entity_id : m_entities) {
            Script* script_component = EM.getComponent<Script>(entity_id);

            // Skip if no script component or if it's not active
            if (!script_component || !script_component->isActive()) {
                continue;
            }

            // Check if this entity's script has been initialized
            if (m_initialized_entities.find(entity_id) == m_initialized_entities.end()) {
                // First time seeing this entity, add its script
                const std::string& script_name = script_component->getScriptName();

                if (!script_name.empty()) {
                    if (addScriptToEntity(entity_id, script_name)) {
                        m_initialized_entities.insert(entity_id);
                        LM.writeLog("ScriptSystem::update() - Script '%s' added to entity %d",
                            script_name.c_str(), entity_id);
                    }
                    else {
                        LM.writeLog("ScriptSystem::update() - Failed to add script '%s' to entity %d",
                            script_name.c_str(), entity_id);
                    }
                }
            }

            // NEW: Update only this specific entity's scripts
            m_script_app->UpdateScriptForEntity(entity_id);
        }
    }

    void ScriptSystem::shutdown() {
        LM.writeLog("ScriptSystem::shutdown() - Shutting down Script System");

        if (m_script_app) {
            m_script_app->ShutdownScripting();
            delete m_script_app;
            m_script_app = nullptr;
        }

        m_initialized_entities.clear();
        m_script_engine_initialized = false;
    }

    void ScriptSystem::process_entity(EntityID entity_id) {
        // Get the Script component
        Script* script_component = EM.getComponent<Script>(entity_id);

        if (!script_component || !script_component->isActive()) {
            return;
        }

        // Check if this entity's script has been initialized
        if (m_initialized_entities.find(entity_id) == m_initialized_entities.end()) {
            // First time seeing this entity, add its script
            const std::string& script_name = script_component->getScriptName();

            if (!script_name.empty()) {
                if (addScriptToEntity(entity_id, script_name)) {
                    m_initialized_entities.insert(entity_id);
                    LM.writeLog("ScriptSystem::process_entity() - Script '%s' added to entity %d",
                        script_name.c_str(), entity_id);
                }
                else {
                    LM.writeLog("ScriptSystem::process_entity() - Failed to add script '%s' to entity %d",
                        script_name.c_str(), entity_id);
                }
            }
        }
    }

    bool ScriptSystem::addScriptToEntity(EntityID entity_id, const std::string& script_name) {
        if (!m_script_engine_initialized || !m_script_app) {
            LM.writeLog("ScriptSystem::addScriptToEntity() - Script engine not initialized");
            return false;
        }

        // Call into the script engine to add the script
        bool success = m_script_app->AddScript(entity_id, script_name.c_str());

        if (success) {
            LM.writeLog("ScriptSystem::addScriptToEntity() - Successfully added script '%s' to entity %d",
                script_name.c_str(), entity_id);
        }
        else {
            LM.writeLog("ScriptSystem::addScriptToEntity() - Failed to add script '%s' to entity %d",
                script_name.c_str(), entity_id);
        }

        return success;
    }

    void ScriptSystem::removeScriptFromEntity(EntityID entity_id) {
        // Remove from tracking
        m_initialized_entities.erase(entity_id);

        // Note: Your current script engine doesn't have a remove function
        // You'll need to add one to EngineInterface if you want to remove scripts at runtime
        LM.writeLog("ScriptSystem::removeScriptFromEntity() - Removed script tracking for entity %d", entity_id);
    }

    void ScriptSystem::reloadAllScripts() {
        if (!m_script_engine_initialized || !m_script_app) {
            return;
        }

        LM.writeLog("ScriptSystem::reloadAllScripts() - Reloading all scripts");

        // Clear tracking
        m_initialized_entities.clear();

        // Reload the script assembly
        m_script_app->ReloadScripts();

        // Re-add all scripts from entities that have Script components
        for (EntityID entity_id : m_entities) {
            Script* script_component = EM.getComponent<Script>(entity_id);

            if (script_component && script_component->isActive()) {
                const std::string& script_name = script_component->getScriptName();
                if (!script_name.empty()) {
                    addScriptToEntity(entity_id, script_name);
                }
            }
        }

        LM.writeLog("ScriptSystem::reloadAllScripts() - Script reload complete");
    }

} // namespace gam300