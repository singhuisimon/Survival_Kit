/**
 * @file ComponentManager.cpp
 * @brief Implementation of the ComponentManager class.
 * @details Contains implementations for all member functions declared in ComponentManager.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ComponentManager.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    // Initialize singleton instance
    ComponentManager::ComponentManager() {
        setType("ComponentManager");
    }

    // Get the singleton instance
    ComponentManager& ComponentManager::getInstance() {
        static ComponentManager instance;
        return instance;
    }

    // Start up the ComponentManager
    int ComponentManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Log startup
        LM.writeLog("ComponentManager::startUp() - Component Manager started successfully");

        return 0;
    }

    // Shut down the ComponentManager
    void ComponentManager::shutDown() {
        // Log shutdown
        LM.writeLog("ComponentManager::shutDown() - Shutting down Component Manager");

        // Clear all component arrays
        m_component_arrays.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Handle entity destruction
    void ComponentManager::entity_destroyed(EntityID entity_id) {
        // Notify each component array that an entity has been destroyed
        for (auto& pair : m_component_arrays) {
            auto& component_array = pair.second;
            component_array->entity_destroyed(entity_id);
        }
    }

} // namespace gam300