/**
 * @file ScriptSystem.h
 * @brief System for managing and executing scripts in the ECS.
 * @details Bridges the ECS with the C# scripting engine.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */
#pragma once
#ifndef __SCRIPT_SYSTEM_H__
#define __SCRIPT_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Script.h"
#include <unordered_set>

 // Forward declaration for the script engine
namespace Core {
    class Application;
}

namespace gam300 {

    /**
     * @brief System for managing script execution in the ECS.
     * @details Processes entities with Script components and coordinates
     *          with the C# scripting engine for script lifecycle management.
     */
    class ScriptSystem : public System {
    private:
        Core::Application* m_script_app;              // Pointer to script engine
        std::unordered_set<EntityID> m_initialized_entities;  // Track which entities have scripts loaded
        bool m_script_engine_initialized;

    public:
        /**
         * @brief Constructor for ScriptSystem.
         */
        ScriptSystem();

        /**
         * @brief Destructor for ScriptSystem.
         */
        ~ScriptSystem() override;

        /**
         * @brief Initialize the script system.
         * @param system_manager Reference to the system manager.
         * @return True if initialization was successful.
         */
        bool init(SystemManager& system_manager) override;

        /**
         * @brief Update all entities with scripts.
         * @param dt Delta time since last update.
         */
        void update(float dt) override;

        /**
         * @brief Clean up the script system.
         */
        void shutdown() override;

        /**
         * @brief Process a specific entity with a script component.
         * @param entity_id The ID of the entity to process.
         */
        void process_entity(EntityID entity_id) override;

        /**
         * @brief Add a script to an entity (called when Script component is added).
         * @param entity_id The entity to add the script to.
         * @param script_name The name of the script class to instantiate.
         * @return True if script was successfully added.
         */
        bool addScriptToEntity(EntityID entity_id, const std::string& script_name);

        /**
         * @brief Remove scripts from an entity (called when Script component is removed).
         * @param entity_id The entity to remove scripts from.
         */
        void removeScriptFromEntity(EntityID entity_id);

        /**
         * @brief Reload all scripts (useful for hot-reloading).
         */
        void reloadAllScripts();

        /**
         * @brief Check if an entity matches the component requirements of this system.
         * @param entity The entity to check.
         * @return True if the entity has all required components.
         */
        bool matches_requirements(const Entity& entity) const override {
            ComponentTypeID script_id = get_component_type_id<Script>();
            return entity.has_component(script_id);
        }
    };

} // namespace gam300

#endif // __SCRIPT_SYSTEM_H__