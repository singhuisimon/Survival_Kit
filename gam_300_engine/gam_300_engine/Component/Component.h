#pragma once
/**
 * @file Component.h
 * @brief Base interface for all components in the Entity Component System.
 * @details Defines the base Component interface that all component types must implement.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "../Utility/ECS_Variables.h"

namespace gam300 {

    /**
     * @brief Base interface for all component types.
     * @details Provides virtual methods that all components must implement.
     */
    class Component {
    public:
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~Component() = default;

        /**
         * @brief Initialize the component after it's been created.
         * @param entity_id The ID of the entity this component is attached to.
         */
        virtual void init(EntityID entity_id) = 0;

        /**
         * @brief Update the component's state.
         * @param dt Delta time - time elapsed since last update.
         */
        virtual void update(float dt) = 0;

        /**
         * @brief Get the entity ID this component is attached to.
         * @return The entity ID.
         */
        EntityID get_owner() const { return m_owner_id; }

    protected:
        EntityID m_owner_id = INVALID_ENTITY_ID; ///< ID of the entity this component is attached to
    };

    /**
     * @brief Template function to get the component type ID at compile time.
     * @details Uses a static counter to generate unique IDs for each component type.
     * @tparam T The component type.
     * @return A unique component type ID.
     */
    template<typename T>
    ComponentTypeID get_component_type_id() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        static ComponentTypeID typeId = next_component_type_id();
        return typeId;
    }

    /**
     * @brief Get the next available component type ID.
     * @details Each call increments and returns a static counter.
     * @return The next available component type ID.
     */
    inline ComponentTypeID next_component_type_id() {
        static ComponentTypeID lastId = 0;
        return lastId++;
    }

} // namespace gam300

#endif // __COMPONENT_H__