#pragma once
/**
 * @file ComponentManager.h
 * @brief Manages component storage and retrieval in the Entity Component System.
 * @details Provides methods to add, remove, and access components attached to entities.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __COMPONENT_MANAGER_H__
#define __COMPONENT_MANAGER_H__

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <vector>
#include "../Component/Component.h"
#include "../Manager/Manager.h"

 // Two-letter acronym for easier access to manager.
#define CM gam300::ComponentManager::getInstance()

namespace gam300 {

    /**
     * @brief Container for a specific type of component.
     * @details Stores and manages all instances of a specific component type.
     */
    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void entity_destroyed(EntityID entity_id) = 0;
    };

    /**
     * @brief Typed container for a specific component type.
     * @details Stores and manages all instances of a specific component type.
     * @tparam T The component type.
     */
    template<typename T>
    class ComponentArray : public IComponentArray {
    public:
        /**
         * @brief Insert a component for an entity.
         * @param entity_id The entity to attach the component to.
         * @param component The component instance.
         */
        void insert_component(EntityID entity_id, T* component) {
            m_entity_to_component[entity_id] = component;
        }

        /**
         * @brief Remove a component from an entity.
         * @param entity_id The entity to remove the component from.
         */
        void remove_component(EntityID entity_id) {
            auto it = m_entity_to_component.find(entity_id);
            if (it != m_entity_to_component.end()) {
                delete it->second;
                m_entity_to_component.erase(it);
            }
        }

        /**
         * @brief Get a component attached to an entity.
         * @param entity_id The entity to get the component from.
         * @return Pointer to the component, or nullptr if not found.
         */
        T* get_component(EntityID entity_id) {
            auto it = m_entity_to_component.find(entity_id);
            if (it != m_entity_to_component.end()) {
                return it->second;
            }
            return nullptr;
        }

        /**
         * @brief Handle entity destruction.
         * @param entity_id The entity that was destroyed.
         */
        void entity_destroyed(EntityID entity_id) override {
            auto it = m_entity_to_component.find(entity_id);
            if (it != m_entity_to_component.end()) {
                delete it->second;
                m_entity_to_component.erase(it);
            }
        }

    private:
        std::unordered_map<EntityID, T*> m_entity_to_component;
    };

    /**
     * @brief Manager for all components in the Entity Component System.
     * @details Provides methods to register component types and manage component instances.
     */
    class ComponentManager : public Manager {
    private:
        ComponentManager();                          // Private since a singleton.
        ComponentManager(ComponentManager const&);   // Don't allow copy.
        void operator=(ComponentManager const&);     // Don't allow assignment.

        // Maps component type IDs to their component arrays
        std::unordered_map<ComponentTypeID, std::shared_ptr<IComponentArray>> m_component_arrays;

    public:
        /**
         * @brief Get the singleton instance of the ComponentManager.
         * @return Reference to the singleton instance.
         */
        static ComponentManager& getInstance();

        /**
         * @brief Start up the ComponentManager.
         * @return 0 on success, -1 on failure.
         */
        int startUp() override;

        /**
         * @brief Shut down the ComponentManager.
         */
        void shutDown() override;

        /**
         * @brief Register a component type with the ComponentManager.
         * @tparam T The component type to register.
         */
        template<typename T>
        void register_component() {
            ComponentTypeID type_id = get_component_type_id<T>();

            // Create a new component array for this type if it doesn't exist
            if (m_component_arrays.find(type_id) == m_component_arrays.end()) {
                m_component_arrays[type_id] = std::make_shared<ComponentArray<T>>();
            }
        }

        /**
         * @brief Create and add a component to an entity.
         * @tparam T The component type to add.
         * @tparam Args Types of arguments to forward to the component constructor.
         * @param entity_id The entity to add the component to.
         * @param args Arguments to forward to the component constructor.
         * @return Pointer to the created component.
         */
        template<typename T, typename... Args>
        T* add_component(EntityID entity_id, Args&&... args) {
            ComponentTypeID type_id = get_component_type_id<T>();

            // Make sure component type is registered
            if (m_component_arrays.find(type_id) == m_component_arrays.end()) {
                register_component<T>();
            }

            // Create the component
            T* component = new T(std::forward<Args>(args)...);
            component->init(entity_id);

            // Add to component array
            auto componentArray = std::static_pointer_cast<ComponentArray<T>>(m_component_arrays[type_id]);
            componentArray->insert_component(entity_id, component);

            return component;
        }

        /**
         * @brief Remove a component from an entity.
         * @tparam T The component type to remove.
         * @param entity_id The entity to remove the component from.
         */
        template<typename T>
        void remove_component(EntityID entity_id) {
            ComponentTypeID type_id = get_component_type_id<T>();

            // Make sure component type is registered
            if (m_component_arrays.find(type_id) != m_component_arrays.end()) {
                auto componentArray = std::static_pointer_cast<ComponentArray<T>>(m_component_arrays[type_id]);
                componentArray->remove_component(entity_id);
            }
        }

        /**
         * @brief Get a component attached to an entity.
         * @tparam T The component type to get.
         * @param entity_id The entity to get the component from.
         * @return Pointer to the component, or nullptr if not found.
         */
        template<typename T>
        T* get_component(EntityID entity_id) {
            ComponentTypeID type_id = get_component_type_id<T>();

            // Make sure component type is registered
            if (m_component_arrays.find(type_id) != m_component_arrays.end()) {
                auto componentArray = std::static_pointer_cast<ComponentArray<T>>(m_component_arrays[type_id]);
                return componentArray->get_component(entity_id);
            }

            return nullptr;
        }

        /**
         * @brief Handle entity destruction.
         * @param entity_id The entity that was destroyed.
         */
        void entity_destroyed(EntityID entity_id);
    };

} // namespace gam300

#endif // __COMPONENT_MANAGER_H__