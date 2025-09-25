/**
 * @file ECSManager.h
 * @brief Declaration of the Entity Component System Manager.
 * @details Manages entities, components, and systems in the ECS architecture.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __ECS_MANAGER_H__
#define __ECS_MANAGER_H__

#include "Manager.h"
#include <vector>
#include <memory>
#include <unordered_map>  // Added for entity name lookup
#include "../Entity/Entity.h"
#include "../Manager/ComponentManager.h"
#include "../System/System.h"

 // Two-letter acronym for easier access to manager.
#define EM gam300::ECSManager::getInstance()

namespace gam300 {

    /**
     * @brief Manager for the Entity Component System.
     * @details Coordinates entities, components, and systems in the ECS architecture.
     */
    class ECSManager : public Manager {
    private:
        ECSManager();                        // Private since a singleton.
        ECSManager(ECSManager const&);       // Don't allow copy.
        void operator=(ECSManager const&);   // Don't allow assignment.

        std::vector<Entity> m_entities;      // Storage for all entities
        EntityID m_next_entity_id;           // Next available entity ID

        // Added: Entity name lookup system
        std::unordered_map<std::string, EntityID> m_entity_name_map;

    public:
        /**
         * @brief Get the singleton instance of the ECSManager.
         * @return Reference to the singleton instance.
         */
        static ECSManager& getInstance();

        /**
         * @brief Start up the ECSManager.
         * @return 0 on success, -1 on failure.
         */
        int startUp() override;

        /**
         * @brief Shut down the ECSManager.
         */
        void shutDown() override;

        /**
         * @brief Create a new entity.
         * @param name Optional name for the entity.
         * @return The created entity.
         */
        Entity& createEntity(const std::string& name = "");

        /**
         * @brief Destroy an entity and remove all its components.
         * @param entity_id The ID of the entity to destroy.
         */
        void destroyEntity(EntityID entity_id);

        /**
         * @brief Get an entity by its ID.
         * @param entity_id The ID of the entity to get.
         * @return Pointer to the entity, or nullptr if not found.
         */
        Entity* getEntity(EntityID entity_id);

        /**
         * @brief Get all entities.
         * @return Reference to the vector of all entities.
         */
        const std::vector<Entity>& getAllEntities() const;

        // =============== ENITY LOOKUP METHODS =============== //

        // Added: Entity lookup methods (Method 1: By name)
        /**
         * @brief Get an entity by its name.
         * @param name The name of the entity to find.
         * @return Pointer to the entity, or nullptr if not found.
         */
        Entity* getEntityByName(const std::string& name);

        /**
         * @brief Get an entity ID by its name.
         * @param name The name of the entity to find.
         * @return The entity ID, or INVALID_ENTITY_ID if not found.
         */
        EntityID getEntityIdByName(const std::string& name);

        /**
         * @brief Check if an entity name already exists.
         * @param name The name to check.
         * @return True if an entity with this name exists, false otherwise.
         */
        bool entityNameExists(const std::string& name);

        // Method 2: By component type(s)
        /**
         * @brief Check if an entity has all specified component types.
         * @tparam T First component type.
         * @tparam Args Additional component types.
         * @param entity_id The entity to check.
         * @return True if entity has all specified components.
         */
        template<typename T, typename... Args>
        bool hasAllComponents(EntityID entity_id) {
            if constexpr (sizeof...(Args) == 0) {
                return hasComponent<T>(entity_id);
            }
            else {
                return hasComponent<T>(entity_id) && hasAllComponents<Args...>(entity_id);
            }
        }

        /**
         * @brief Find all entities that have a specific component type.
         * @tparam T The component type to search for.
         * @return Vector of entity IDs that have the specified component.
         */
        template<typename T>
        std::vector<EntityID> getEntitiesWithComponent() {
            std::vector<EntityID> result;
            ComponentTypeID component_id = get_component_type_id<T>();

            for (const auto& entity : m_entities) {
                if (entity.has_component(component_id)) {
                    result.push_back(entity.get_id());
                }
            }
            return result;
        }

        /**
         * @brief Find all entities that have multiple specific component types.
         * @tparam T First component type.
         * @tparam Args Additional component types.
         * @return Vector of entity IDs that have ALL specified components.
         */
        template<typename T, typename... Args>
        std::vector<EntityID> getEntitiesWithComponents() {
            std::vector<EntityID> result;

            for (const auto& entity : m_entities) {
                if (hasAllComponents<T, Args...>(entity.get_id())) {
                    result.push_back(entity.get_id());
                }
            }
            return result;
        }

        /**
         * @brief Find the first entity with a specific component type.
         * @tparam T The component type to search for.
         * @return Entity ID of first entity with component, or INVALID_ENTITY_ID if none found.
         */
        template<typename T>
        EntityID getFirstEntityWithComponent() {
            ComponentTypeID component_id = get_component_type_id<T>();

            for (const auto& entity : m_entities) {
                if (entity.has_component(component_id)) {
                    return entity.get_id();
                }
            }
            return INVALID_ENTITY_ID;
        }

        /**
         * @brief Find entities by component type and name pattern.
         * @tparam T The component type to search for.
         * @param namePattern String that entity name must contain.
         * @return Vector of entity IDs matching both criteria.
         */
        template<typename T>
        std::vector<EntityID> getEntitiesWithComponentAndName(const std::string& namePattern) {
            std::vector<EntityID> result;
            ComponentTypeID component_id = get_component_type_id<T>();

            for (const auto& entity : m_entities) {
                if (entity.has_component(component_id) &&
                    entity.get_name().find(namePattern) != std::string::npos) {
                    result.push_back(entity.get_id());
                }
            }
            return result;
        }

        /**
         * @brief Count entities with a specific component type.
         * @tparam T The component type to count.
         * @return Number of entities with the specified component.
         */
        template<typename T>
        size_t countEntitiesWithComponent() {
            size_t count = 0;
            ComponentTypeID component_id = get_component_type_id<T>();

            for (const auto& entity : m_entities) {
                if (entity.has_component(component_id)) {
                    count++;
                }
            }
            return count;
        }

        // =============== END ENTITY LOOKUP METHODS =============== //

        /**
         * @brief Clear all entities from the ECS.
         * @details Destroys all entities and clears internal storage.
         */
        void clearAllEntities();

        /**
         * @brief Rename an existing entity.
         * @param entity_id The ID of the entity to rename.
         * @param new_name The new name for the entity.
         */
        void renameEntity(EntityID entity_id, const std::string& new_name);

        /**
         * @brief Add a component to an entity.
         * @tparam T The component type to add.
         * @tparam Args Types of arguments to forward to the component constructor.
         * @param entity_id The ID of the entity to add the component to.
         * @param args Arguments to forward to the component constructor.
         * @return Pointer to the newly created component.
         */
        template<typename T, typename... Args>
        T* addComponent(EntityID entity_id, Args&&... args) {
            // Find the entity
            Entity* entity = getEntity(entity_id);
            if (!entity) {
                return nullptr;
            }

            // Add the component type to the entity's mask
            ComponentTypeID component_id = get_component_type_id<T>();
            entity->add_component(component_id);

            // Add the component to the ComponentManager
            T* component = CM.add_component<T>(entity_id, std::forward<Args>(args)...);

            // Notify the SystemManager that the entity's components changed
            SM.entity_components_changed(*entity);

            return component;
        }

        /**
         * @brief Remove a component from an entity.
         * @tparam T The component type to remove.
         * @param entity_id The ID of the entity to remove the component from.
         */
        template<typename T>
        void removeComponent(EntityID entity_id) {
            // Find the entity
            Entity* entity = getEntity(entity_id);
            if (!entity) {
                return;
            }

            // Remove the component type from the entity's mask
            ComponentTypeID component_id = get_component_type_id<T>();
            entity->remove_component(component_id);

            // Remove the component from the ComponentManager
            CM.remove_component<T>(entity_id);

            // Notify the SystemManager that the entity's components changed
            SM.entity_components_changed(*entity);
        }

        /**
         * @brief Get a component from an entity.
         * @tparam T The component type to get.
         * @param entity_id The ID of the entity to get the component from.
         * @return Pointer to the component, or nullptr if not found.
         */
        template<typename T>
        T* getComponent(EntityID entity_id) {
            return CM.get_component<T>(entity_id);
        }

        /**
         * @brief Check if an entity has a specific component.
         * @tparam T The component type to check for.
         * @param entity_id The ID of the entity to check.
         * @return True if the entity has the component, false otherwise.
         */
        template<typename T>
        bool hasComponent(EntityID entity_id) {
            Entity* entity = getEntity(entity_id);
            if (!entity) {
                return false;
            }

            ComponentTypeID component_id = get_component_type_id<T>();
            return entity->has_component(component_id);
        }

        /**
         * @brief Register a system with the ECS.
         * @tparam T The system type to register.
         * @tparam Args Types of arguments to forward to the system constructor.
         * @param args Arguments to forward to the system constructor.
         * @return Shared pointer to the created system.
         */
        template<typename T, typename... Args>
        std::shared_ptr<T> registerSystem(Args&&... args) {
            return SM.register_system<T>(std::forward<Args>(args)...);
        }

        /**
         * @brief Get a system of a specific type.
         * @tparam T The system type to get.
         * @return Shared pointer to the system, or nullptr if not found.
         */
        template<typename T>
        std::shared_ptr<T> getSystem() {
            return SM.get_system<T>();
        }

        /**
         * @brief Update all systems.
         * @param dt Delta time in seconds.
         */
        void updateSystems(float dt);
    };

} // namespace gam300

#endif // __ECS_MANAGER_H__