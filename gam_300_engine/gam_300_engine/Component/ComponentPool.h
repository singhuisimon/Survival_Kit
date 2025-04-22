/**
 * @file ComponentPool.h
 * @brief Provides a contiguous memory storage for components of the same type.
 * @details Implements a component pool with O(1) access and efficient cache utilization.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __COMPONENT_POOL_H__
#define __COMPONENT_POOL_H__

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <memory>
#include "../Utility/ECS_Variables.h"

namespace gam300 {

    /**
     * @brief Provides contiguous storage for components of a single type.
     * @details Uses a packed array approach for better cache locality with
     *          O(1) access by entity ID. Components are stored densely to
     *          allow for efficient iteration.
     * @tparam T The component type stored in this pool.
     */
    template<typename T>
    class ComponentPool {
    public:
        /**
         * @brief Constructor that optionally pre-allocates memory.
         * @param initial_capacity Initial capacity to reserve (default: 100).
         */
        ComponentPool(size_t initial_capacity = 100) {
            m_components.reserve(initial_capacity);
        }

        /**
         * @brief Insert a component for an entity.
         * @param entity_id The entity to add the component to.
         * @param component Unique pointer to the component to add.
         * @return Pointer to the stored component.
         */
        T* insert(EntityID entity_id, std::unique_ptr<T> component) {
            // If entity already has a component of this type, replace it
            auto it = m_entity_to_index.find(entity_id);
            if (it != m_entity_to_index.end()) {
                // Replace existing component
                size_t index = it->second;
                m_components[index] = std::move(component);
                return m_components[index].get();
            }

            // Add new component
            size_t new_index = m_components.size();
            m_entity_to_index[entity_id] = new_index;
            m_index_to_entity[new_index] = entity_id;
            m_components.push_back(std::move(component));
            return m_components.back().get();
        }

        /**
         * @brief Remove a component from an entity.
         * @param entity_id The entity to remove the component from.
         * @return True if component was removed, false if entity had no component.
         */
        bool remove(EntityID entity_id) {
            auto it = m_entity_to_index.find(entity_id);
            if (it == m_entity_to_index.end()) {
                return false; // Entity doesn't have this component
            }

            // Get the index of the component to remove
            size_t index_to_remove = it->second;
            size_t last_index = m_components.size() - 1;

            // If it's not the last element, move the last element to this position
            if (index_to_remove < last_index) {
                // Move the last component to the removed position
                m_components[index_to_remove] = std::move(m_components[last_index]);

                // Update the indices for the moved entity
                EntityID moved_entity = m_index_to_entity[last_index];
                m_entity_to_index[moved_entity] = index_to_remove;
                m_index_to_entity[index_to_remove] = moved_entity;
            }

            // Remove the last element and update maps
            m_components.pop_back();
            m_entity_to_index.erase(entity_id);
            m_index_to_entity.erase(last_index);

            return true;
        }

        /**
         * @brief Get a component attached to an entity.
         * @param entity_id The entity to get the component from.
         * @return Pointer to the component, or nullptr if not found.
         */
        T* get(EntityID entity_id) {
            auto it = m_entity_to_index.find(entity_id);
            if (it != m_entity_to_index.end()) {
                return m_components[it->second].get();
            }
            return nullptr;
        }

        /**
         * @brief Check if an entity has a component in this pool.
         * @param entity_id The entity to check.
         * @return True if the entity has a component, false otherwise.
         */
        bool has(EntityID entity_id) const {
            return m_entity_to_index.find(entity_id) != m_entity_to_index.end();
        }

        /**
         * @brief Get the number of components in the pool.
         * @return The number of components.
         */
        size_t size() const {
            return m_components.size();
        }

        /**
         * @brief Clear all components from the pool.
         */
        void clear() {
            m_components.clear();
            m_entity_to_index.clear();
            m_index_to_entity.clear();
        }

        /**
         * @brief Get all components for iteration.
         * @return Reference to the vector of components.
         */
        const std::vector<std::unique_ptr<T>>& get_components() const {
            return m_components;
        }

        /**
         * @brief Get entity ID for a component at a specific index.
         * @param index The index in the component array.
         * @return The entity ID associated with that component.
         */
        EntityID get_entity_at(size_t index) const {
            auto it = m_index_to_entity.find(index);
            if (it != m_index_to_entity.end()) {
                return it->second;
            }
            return INVALID_ENTITY_ID;
        }

    private:
        std::vector<std::unique_ptr<T>> m_components;              ///< Dense array of components
        std::unordered_map<EntityID, size_t> m_entity_to_index;    ///< Maps entity IDs to component indices
        std::unordered_map<size_t, EntityID> m_index_to_entity;    ///< Maps component indices to entity IDs
    };

} // namespace gam300

#endif // __COMPONENT_POOL_H__