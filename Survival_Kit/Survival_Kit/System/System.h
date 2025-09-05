/**
 * @file System.h
 * @brief Base System class and system manager for the Entity Component System.
 * @details Defines the interface for processing entities with specific component combinations.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <memory>
#include <algorithm>
#include "../Component/Component.h"
#include "../Manager/ComponentManager.h"
#include "../Entity/Entity.h"
#include "../Manager/Manager.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    /**
     * @brief Forward declaration of the SystemManager
     */
    class SystemManager;

    /**
     * @brief Base class for all systems in the ECS.
     * @details Systems process entities that have specific combinations of components.
     */
    class System {
    public:
        /**
         * @brief Constructor for the System class.
         * @param name The name of the system for identification and debugging.
         */
        System(const std::string& name)
            : m_name(name), m_is_active(true), m_priority(0) {}

        /**
         * @brief Virtual destructor for proper cleanup of derived classes.
         */
        virtual ~System() = default;

        /**
         * @brief Initialize the system.
         * @param system_manager Reference to the system manager.
         * @return True if initialization was successful, false otherwise.
         */
        virtual bool init(SystemManager& system_manager) = 0;

        /**
         * @brief Update the system, processing all relevant entities.
         * @param dt Delta time since the last update.
         */
        virtual void update(float dt) = 0;

        /**
         * @brief Clean up the system when shutting down.
         */
        virtual void shutdown() = 0;

        /**
         * @brief Process a specific entity if it matches the system's requirements.
         * @param entity_id The ID of the entity to process.
         */
        virtual void process_entity(EntityID entity_id) = 0;

        /**
         * @brief Add an entity to be processed by this system.
         * @param entity_id The ID of the entity to add.
         */
        void add_entity(EntityID entity_id) {
            // Only add the entity if it's not already in the list
            if (std::find(m_entities.begin(), m_entities.end(), entity_id) == m_entities.end()) {
                m_entities.push_back(entity_id);
            }
        }

        /**
         * @brief Remove an entity from being processed by this system.
         * @param entity_id The ID of the entity to remove.
         */
        void remove_entity(EntityID entity_id) {
            m_entities.erase(
                std::remove(m_entities.begin(), m_entities.end(), entity_id),
                m_entities.end()
            );
        }

        /**
         * @brief Get whether the system is active.
         * @return True if the system is active, false otherwise.
         */
        bool is_active() const {
            return m_is_active;
        }

        /**
         * @brief Set whether the system is active.
         * @param active The new active state.
         */
        void set_active(bool active) {
            m_is_active = active;
        }

        /**
         * @brief Get the system's name.
         * @return The name of the system.
         */
        const std::string& get_name() const {
            return m_name;
        }

        /**
         * @brief Get the system's update priority.
         * @details Higher priority systems get updated first.
         * @return The priority value.
         */
        int get_priority() const {
            return m_priority;
        }

        /**
         * @brief Set the system's update priority.
         * @details Higher priority systems get updated first.
         * @param priority The new priority value.
         */
        void set_priority(int priority) {
            m_priority = priority;
        }

        /**
         * @brief Check if an entity matches the component requirements of this system.
         * @param entity The entity to check.
         * @return True if the entity has all the required components, false otherwise.
         */
        virtual bool matches_requirements(const Entity& entity) const = 0;

        /**
         * @brief Get the list of entities managed by this system.
         * @return Vector of entity IDs processed by this system.
         */
        const std::vector<EntityID>& get_entities() const {
            return m_entities;
        }

        /**
         * @brief Check if this system contains a specific entity.
         * @param entity_id The entity ID to check.
         * @return True if the entity is in this system, false otherwise.
         */
        bool has_entity(EntityID entity_id) const {
            return std::find(m_entities.begin(), m_entities.end(), entity_id) != m_entities.end();
        }

    protected:
        std::string m_name;              ///< Name of the system
        std::vector<EntityID> m_entities; ///< Entities processed by this system
        bool m_is_active;                ///< Whether the system is active
        int m_priority;                  ///< Update priority (higher = updated earlier)
    };

    /**
     * @brief Manager class for all systems in the ECS.
     * @details Handles system registration, updates, and entity management.
     */
    class SystemManager : public Manager {
    private:
        SystemManager();                           // Private since a singleton.
        SystemManager(SystemManager const&);       // Don't allow copy.
        void operator=(SystemManager const&);      // Don't allow assignment.

        std::vector<std::shared_ptr<System>> m_systems; ///< All registered systems
        std::unordered_map<std::type_index, std::shared_ptr<System>> m_system_types; ///< Map of system types to instances

    public:
        /**
         * @brief Get the singleton instance of the SystemManager.
         * @return Reference to the singleton instance.
         */
        static SystemManager& getInstance();

        /**
         * @brief Start up the SystemManager.
         * @return 0 on success, -1 on failure.
         */
        int startUp() override;

        /**
         * @brief Shut down the SystemManager.
         */
        void shutDown() override;

        /**
         * @brief Register a system with the SystemManager.
         * @tparam T The system type to register.
         * @tparam Args Types of arguments to forward to the system constructor.
         * @param args Arguments to forward to the system constructor.
         * @return Shared pointer to the created system.
         */
        template<typename T, typename... Args>
        std::shared_ptr<T> register_system(Args&&... args) {
            // Check if system type is already registered
            auto type_index = std::type_index(typeid(T));
            if (m_system_types.find(type_index) != m_system_types.end()) {
                // Return existing system
                return std::static_pointer_cast<T>(m_system_types[type_index]);
            }

            // Create a new system
            auto system = std::make_shared<T>(std::forward<Args>(args)...);
            if (!system->init(*this)) {
                LM.writeLog("Failed to initialize system: %s", system->get_name().c_str());
                return nullptr;
            }

            // Store the system
            m_systems.push_back(system);
            m_system_types[type_index] = system;

            // Sort systems by priority
            sort_systems();

            LM.writeLog("Registered system: %s", system->get_name().c_str());
            return system;
        }

        /**
         * @brief Get a system of a specific type.
         * @tparam T The system type to get.
         * @return Shared pointer to the system, or nullptr if not found.
         */
        template<typename T>
        std::shared_ptr<T> get_system() {
            auto type_index = std::type_index(typeid(T));
            auto it = m_system_types.find(type_index);
            if (it != m_system_types.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
            return nullptr;
        }

        /**
         * @brief Update all systems.
         * @param dt Delta time since the last update.
         */
        void update_systems(float dt);

        /**
         * @brief Sort systems by priority.
         */
        void sort_systems();

        /**
         * @brief Entity was created, notify all systems.
         * @param entity The entity that was created.
         */
        void entity_created(const Entity& entity);

        /**
         * @brief Entity was destroyed, notify all systems.
         * @param entity_id The ID of the entity that was destroyed.
         */
        void entity_destroyed(EntityID entity_id);

        /**
         * @brief Entity's component mask changed, notify all systems.
         * @param entity The entity whose component mask changed.
         */
        void entity_components_changed(const Entity& entity);
    };

    // Define the SM macro for easier access to the SystemManager
#define SM gam300::SystemManager::getInstance()

/**
 * @brief Base template for systems that require specific components.
 * @tparam Components The component types required by this system.
 */
    template<typename... Components>
    class ComponentSystem : public System {
    public:
        /**
         * @brief Constructor for the ComponentSystem.
         * @param name The name of the system.
         */
        ComponentSystem(const std::string& name) : System(name) {
            // Create the component mask for this system by setting bits for each component type
            (m_component_mask.set(get_component_type_id<Components>()), ...);
        }

        /**
         * @brief Check if an entity matches the component requirements of this system.
         * @param entity The entity to check.
         * @return True if the entity has all the required components, false otherwise.
         */
        bool matches_requirements(const Entity& entity) const override {
            // An entity matches if all the bits in the system's component mask are set in the entity's mask
            return (entity.get_component_mask() & m_component_mask) == m_component_mask;
        }

    protected:
        ComponentMask m_component_mask; ///< Bit mask of required components
    };

} // namespace gam300

#endif // __SYSTEM_H__