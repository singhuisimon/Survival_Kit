/**
 * @file SystemManager.cpp
 * @brief Implementation of the SystemManager class.
 * @details Contains implementations for all member functions declared in System.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../System/System.h"

namespace gam300 {

    // Initialize singleton instance
    SystemManager::SystemManager() {
        setType("SystemManager");
    }

    // Get the singleton instance
    SystemManager& SystemManager::getInstance() {
        static SystemManager instance;
        return instance;
    }

    // Start up the SystemManager
    int SystemManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Log startup
        LM.writeLog("SystemManager::startUp() - System Manager started successfully");

        return 0;
    }

    // Shut down the SystemManager
    void SystemManager::shutDown() {
        // Log shutdown
        LM.writeLog("SystemManager::shutDown() - Shutting down System Manager");

        // Shut down all systems in reverse order of priority
        for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
            (*it)->shutdown();
        }

        // Clear all systems
        m_systems.clear();
        m_system_types.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Update all systems
    void SystemManager::update_systems(float dt) {
        // Only update active systems
        for (auto& system : m_systems) {
            if (system->is_active()) {
                system->update(dt);
            }
        }
    }

    // Sort systems by priority
    void SystemManager::sort_systems() {
        // Sort in descending order (higher priority first)
        std::sort(m_systems.begin(), m_systems.end(),
            [](const std::shared_ptr<System>& a, const std::shared_ptr<System>& b) {
                return a->get_priority() > b->get_priority();
            });
    }

    // Entity was created, notify all systems
    void SystemManager::entity_created(const Entity& entity) {
        // Check each system to see if the entity should be added
        for (auto& system : m_systems) {
            if (system->matches_requirements(entity)) {
                system->add_entity(entity.get_id());
            }
        }
    }

    // Entity was destroyed, notify all systems
    void SystemManager::entity_destroyed(EntityID entity_id) {
        // Remove entity from all systems
        for (auto& system : m_systems) {
            system->remove_entity(entity_id);
        }
    }

    // Entity's component mask changed, notify all systems
    void SystemManager::entity_components_changed(const Entity& entity) {
        // Check each system to see if the entity should be added or removed
        for (auto& system : m_systems) {
            bool matches = system->matches_requirements(entity);
            bool has_entity = system->has_entity(entity.get_id());

            if (matches && !has_entity) {
                // Entity should be added to the system
                system->add_entity(entity.get_id());
            }
            else if (!matches && has_entity) {
                // Entity should be removed from the system
                system->remove_entity(entity.get_id());
            }
        }
    }

} // namespace gam300