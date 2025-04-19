/**
 * @file ComponentView.h
 * @brief Provides efficient iteration over entities with specific component combinations.
 * @details Allows systems to easily iterate over entities that have all required components.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __COMPONENT_VIEW_H__
#define __COMPONENT_VIEW_H__

#include <vector>
#include <unordered_set>
#include <tuple>
#include <functional>
#include "../Utility/ECS_Variables.h"
#include "../Manager/ComponentManager.h"

namespace gam300 {

    /**
     * @brief Provides efficient iteration over entities with specific component combinations.
     * @tparam Components The component types to iterate over.
     */
    template<typename... Components>
    class ComponentView {
    public:
        /**
         * @brief Constructor that builds a view of entities with specified components.
         * @details Finds all entities that have all of the specified component types.
         */
        ComponentView() {
            // Build the list of entities that have all required components
            update_entity_list();
        }

        /**
         * @brief Updates the internal list of entities with the required components.
         * @details Should be called when entities or components change if view is long-lived.
         */
        void update_entity_list() {
            m_entities.clear();

            // Choose the component type with the fewest instances to iterate over
            find_entities_with_components<Components...>();
        }

        /**
         * @brief Performs a function on each entity and its components.
         * @param func The function to execute for each entity.
         */
        template<typename Func>
        void each(Func&& func) const {
            for (EntityID entity : m_entities) {
                // Call the function with the entity and its components
                invoke_with_components(entity, std::forward<Func>(func));
            }
        }

        /**
         * @brief Get the number of entities in this view.
         * @return The number of entities.
         */
        size_t size() const {
            return m_entities.size();
        }

        /**
         * @brief Check if the view is empty.
         * @return True if there are no entities in this view.
         */
        bool empty() const {
            return m_entities.empty();
        }

        /**
         * @brief Get a vector of all entity IDs in this view.
         * @return Vector of entity IDs.
         */
        const std::vector<EntityID>& entities() const {
            return m_entities;
        }

    private:
        std::vector<EntityID> m_entities;  ///< List of entities with all required components

        // Helper to find the component type with the fewest instances
        template<typename First, typename... Rest>
        void find_entities_with_components() {
            ComponentTypeID type_id = get_component_type_id<First>();
            auto component_array = std::static_pointer_cast<ComponentArray<First>>(
                CM.m_component_arrays[type_id]);

            // Get entities for the first component type
            for (size_t i = 0; i < component_array->size(); ++i) {
                EntityID entity = component_array->get_entity_at(i);

                // Check if this entity has all the other component types
                if (has_all_components<Rest...>(entity)) {
                    m_entities.push_back(entity);
                }
            }
        }

        // Base case for the recursion when no component types are left to check
        template<typename... None>
        typename std::enable_if<sizeof...(None) == 0, bool>::type
            has_all_components(EntityID /*entity*/) const {
            return true;
        }

        // Recursive check for whether an entity has all the specified component types
        template<typename First, typename... Rest>
        bool has_all_components(EntityID entity) const {
            if (CM.get_component<First>(entity) == nullptr) {
                return false;
            }
            return has_all_components<Rest...>(entity);
        }

        // Helper to invoke a function with entity and components
        template<typename Func>
        void invoke_with_components(EntityID entity, Func&& func) const {
            // Call the function with entity and components as arguments
            call_with_components<Func, Components...>(entity, std::forward<Func>(func));
        }

        // Helper to call a function with an entity and its components
        template<typename Func, typename... CompTypes>
        void call_with_components(EntityID entity, Func&& func) const {
            func(entity, *CM.get_component<CompTypes>(entity)...);
        }
    };

    /**
     * @brief Creates a view for iterating over entities with specific component types.
     * @tparam Components The component types to include in the view.
     * @return A ComponentView for the specified component types.
     */
    template<typename... Components>
    ComponentView<Components...> create_view() {
        return ComponentView<Components...>();
    }

} // namespace gam300

#endif // __COMPONENT_VIEW_H__