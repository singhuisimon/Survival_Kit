/**
 * @file ECSManager.cpp
 * @brief Implementation of the Entity Component System Manager.
 * @details Contains implementations for all member functions declared in ECSManager.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ECSManager.h"
#include "LogManager.h"
#include <algorithm>

#include "../System/AudioSystem.h"

namespace gam300 {

    // Initialize singleton instance
    ECSManager::ECSManager() {
        setType("ECSManager");
        m_next_entity_id = 0;
    }

    // Get the singleton instance
    ECSManager& ECSManager::getInstance() {
        static ECSManager instance;
        return instance;
    }

    // Start up the ECSManager
    int ECSManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Start the ComponentManager
        if (CM.startUp()) {
            LM.writeLog("ECSManager::startUp() - Failed to start ComponentManager");
            return -1;
        }

        LM.writeLog("ECSManager::startUp() - ComponentManager started successfully");

        // Start the SystemManager
        if (SM.startUp()) {
            LM.writeLog("ECSManager::startUp() - Failed to start SystemManager");
            CM.shutDown();
            return -1;
        }

        LM.writeLog("ECSManager::startUp() - SystemManager started successfully");
        
        // register audio system
        auto audioSystem = EM.registerSystem<AudioSystem>();
        if (!audioSystem) {
            LM.writeLog("ECSManager::startUp() - Failed to register AudioSystem");
        }
        else {
            LM.writeLog("ECSManager::startUp() - AudioSystem registered successfully");
        }
        
        LM.writeLog("ECSManager::startUp() - ECS Manager started successfully");

        return 0;
    }

    // Shut down the ECSManager
    void ECSManager::shutDown() {
        LM.writeLog("ECSManager::shutDown() - Shutting down ECS Manager");

        // Destroy all entities first (this will also clear the name map)
        clearAllEntities();

        // Shut down managers in reverse order of initialization
        SM.shutDown();
        CM.shutDown();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Create a new entity
    Entity& ECSManager::createEntity(const std::string& name) {
        // Generate a new entity ID
        EntityID id = m_next_entity_id++;

        // Handle name conflicts if name is provided
        std::string finalName = name;
        if (!name.empty()) {
            if (entityNameExists(name)) {
                // Generate unique name by appending number
                int counter = 1;
                do {
                    finalName = name + "_" + std::to_string(counter);
                    counter++;
                } while (entityNameExists(finalName));

                LM.writeLog("ECSManager::createEntity() - Name conflict resolved: '%s' -> '%s'",
                    name.c_str(), finalName.c_str());
            }
        }

        // Create a new entity and add it to the list
        m_entities.emplace_back(id, finalName);
        Entity& entity = m_entities.back();

        // Add to name lookup map if name is provided
        if (!finalName.empty()) {
            m_entity_name_map[finalName] = id;
        }

        // Notify the SystemManager about the new entity
        SM.entity_created(entity);

        // Log the creation
        LM.writeLog("ECSManager::createEntity() - Created entity %d with name '%s'",
            id, finalName.empty() ? "(unnamed)" : finalName.c_str());

        return entity;
    }

    // Destroy an entity
    void ECSManager::destroyEntity(EntityID entity_id) {
        // Find the entity
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
            [entity_id](const Entity& e) { return e.get_id() == entity_id; });

        if (it != m_entities.end()) {
            // Get the name for logging and cleanup
            std::string name = it->get_name();

            // Remove from name map if it has a name
            if (!name.empty()) {
                m_entity_name_map.erase(name);
            }

            // Notify the SystemManager that the entity is being destroyed
            SM.entity_destroyed(entity_id);

            // Notify the ComponentManager that the entity is being destroyed
            CM.entity_destroyed(entity_id);

            // Remove the entity from our list
            m_entities.erase(it);

            // Log the destruction
            LM.writeLog("ECSManager::destroyEntity() - Destroyed entity %d with name '%s'",
                entity_id, name.empty() ? "(unnamed)" : name.c_str());
        }
        else {
            LM.writeLog("ECSManager::destroyEntity() - WARNING: Entity %d not found", entity_id);
        }
    }

    // Get an entity by ID
    Entity* ECSManager::getEntity(EntityID entity_id) {
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
            [entity_id](const Entity& e) { return e.get_id() == entity_id; });

        return (it != m_entities.end()) ? &(*it) : nullptr;
    }

    // Get all entities
    const std::vector<Entity>& ECSManager::getAllEntities() const {
        return m_entities;
    }

    // Get an entity by its name
    Entity* ECSManager::getEntityByName(const std::string& name) {
        auto it = m_entity_name_map.find(name);
        if (it != m_entity_name_map.end()) {
            return getEntity(it->second);
        }
        return nullptr;
    }

    // Get an entity ID by its name
    EntityID ECSManager::getEntityIdByName(const std::string& name) {
        auto it = m_entity_name_map.find(name);
        if (it != m_entity_name_map.end()) {
            return it->second;
        }
        return INVALID_ENTITY_ID;
    }

    // Check if an entity name already exists
    bool ECSManager::entityNameExists(const std::string& name) {
        return m_entity_name_map.find(name) != m_entity_name_map.end();
    }

    // Clear all entities from the ECS
    void ECSManager::clearAllEntities() {
        LM.writeLog("ECSManager::clearAllEntities() - Clearing %zu entities", m_entities.size());

        // Destroy all entities properly to ensure cleanup
        std::vector<EntityID> entityIds;
        for (const auto& entity : m_entities) {
            entityIds.push_back(entity.get_id());
        }

        for (EntityID id : entityIds) {
            destroyEntity(id);
        }

        // Ensure everything is clean (should already be empty after destroyEntity calls)
        // Reset m_next_entity_id to 0 after clearing all entities (Edited - Lily (21/9))
        m_entities.clear();
        m_entity_name_map.clear();
        m_next_entity_id = 0;

        LM.writeLog("ECSManager::clearAllEntities() - All entities cleared");
    }

    // Rename an existing entity
    void ECSManager::renameEntity(EntityID entity_id, const std::string& new_name) {
        Entity* entity = getEntity(entity_id);
        if (!entity) {
            LM.writeLog("ECSManager::renameEntity() - Entity %d not found", entity_id);
            return;
        }

        std::string oldName = entity->get_name();

        // Remove old name from map
        if (!oldName.empty()) {
            m_entity_name_map.erase(oldName);
        }

        // Check for name conflicts and resolve them
        std::string finalName = new_name;
        if (!new_name.empty() && entityNameExists(new_name)) {
            int counter = 1;
            do {
                finalName = new_name + "_" + std::to_string(counter);
                counter++;
            } while (entityNameExists(finalName));

            LM.writeLog("ECSManager::renameEntity() - Name conflict resolved: '%s' -> '%s'",
                new_name.c_str(), finalName.c_str());
        }

        // Update entity name
        entity->set_name(finalName);

        // Add new name to map
        if (!finalName.empty()) {
            m_entity_name_map[finalName] = entity_id;
        }

        LM.writeLog("ECSManager::renameEntity() - Entity %d renamed from '%s' to '%s'",
            entity_id, oldName.c_str(), finalName.c_str());
    }

    // Update all systems
    void ECSManager::updateSystems(float dt) {
        SM.update_systems(dt);
    }

} // namespace gam300