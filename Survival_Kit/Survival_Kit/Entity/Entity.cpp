/**
 * @file Entity.cpp
 * @brief Implementation of the Entity class for the Entity Component System (ECS).
 * @author Simon Chan (100%)
 * @date September 15, 2024
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Entity.h"

namespace gam300 {

    // Constructor
    Entity::Entity(EntityID id, const std::string& name)
        : id(id), name(name) {
        // ComponentMask is initialized with all bits set to 0 by default
    }

    // Get the unique identifier
    EntityID Entity::get_id() const {
        return id;
    }

    // Get the entity name
    const std::string& Entity::get_name() const {
        return name;
    }

    // Set the entity name
    void Entity::set_name(const std::string& new_name) {
        name = new_name;
    }

    // Add a component to the entity
    void Entity::add_component(std::size_t component_id) {
        mask.set(component_id);
    }

    // Remove a component from the entity
    void Entity::remove_component(std::size_t component_id) {
        mask.reset(component_id);
    }

    // Check if the entity has a specific component
    bool Entity::has_component(std::size_t component_id) const {
        return mask.test(component_id);
    }

    // Get the component mask
    ComponentMask Entity::get_component_mask() const {
        return mask;
    }

} // namespace GAM300