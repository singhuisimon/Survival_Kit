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
        LM.writeLog("ECSManager::startUp() - ECS Manager started successfully");

        return 0;
    }

    // Shut down the ECSManager
    void ECSManager::shutDown() {
        LM.writeLog("ECSManager::shutDown() - Shutting down ECS Manager");

        // Destroy all entities first
        m_entities.clear();

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

        // Create a new entity and add it to the list
        m_entities.emplace_back(id, name);
        Entity& entity = m_entities.back();

        // Notify the SystemManager about the new entity
        SM.entity_created(entity);

        // Log the creation
        LM.writeLog("ECSManager::createEntity() - Created entity %d with name '%s'",
            id, name.empty() ? "(unnamed)" : name.c_str());

        return entity;
    }

    // Destroy an entity
    void ECSManager::destroyEntity(EntityID entity_id) {
        // Find the entity
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
            [entity_id](const Entity& e) { return e.get_id() == entity_id; });

        if (it != m_entities.end()) {
            // Get the name for logging
            std::string name = it->get_name();

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

    // Update all systems
    void ECSManager::updateSystems(float dt) {
        SM.update_systems(dt);
    }

} // namespace gam300