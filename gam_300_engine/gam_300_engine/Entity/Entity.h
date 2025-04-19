#pragma once
/**
 * @file Entity.h
 * @brief Defines the Entity class for the Entity Component System (ECS).
 * @author Simon Chan (100%)
 * @date September 15, 2024
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef GAM300_ENTITY_H
#define GAM300_ENTITY_H
 // Include standard headers
#include <cstdint>
#include <bitset>
#include <string>

// Include other necessary headers
#include "../Utility/ECS_Variables.h" // For MAX_COMPONENTS, EntityID, and ComponentMask

namespace gam300 {
    /**
     * @brief Represents a game object in the Entity Component System.
     * @details An Entity is essentially just an ID and a mask of components. It doesn't
     *          store component data directly, but rather acts as a handle to access
     *          components stored elsewhere in the ECS.
     */
    class Entity {
    private:
        EntityID id;           ///< Unique identifier for the entity
        ComponentMask mask;    ///< Bitset indicating which components the entity has
        std::string name;      ///< Name of the entity from the scene file

    public:
        /**
         * @brief Constructor for Entity.
         * @param id Unique identifier for the new entity.
         * @param name Optional name for the entity.
         */
        Entity(EntityID id, const std::string& name = "");

        /**
         * @brief Get the unique identifier of the entity.
         * @return The entity's unique identifier.
         */
        EntityID get_id() const;

        /**
         * @brief Get the name of the entity.
         * @return The entity's name.
         */
        const std::string& get_name() const;

        /**
         * @brief Set the name of the entity.
         * @param new_name The new name for the entity.
         */
        void set_name(const std::string& new_name);

        /**
         * @brief Add a component to the entity.
         * @details Sets the bit corresponding to the component_id in the mask.
         * @param component_id The ID of the component type to add.
         */
        void add_component(std::size_t component_id);

        /**
         * @brief Remove a component from the entity.
         * @details Resets the bit corresponding to the component_id in the mask.
         * @param component_id The ID of the component type to remove.
         */
        void remove_component(std::size_t component_id);

        /**
         * @brief Check if the entity has a specific component.
         * @details Tests the bit corresponding to the component_id in the mask.
         * @param component_id The ID of the component type to check for.
         * @return True if the entity has the component, false otherwise.
         */
        bool has_component(std::size_t component_id) const;

        /**
         * @brief Get the component mask of the entity.
         * @details Returns the entire component mask, which can be useful for
         *          system queries in the ECS.
         * @return The entity's component mask.
         */
        ComponentMask get_component_mask() const;

        /**
         * @brief Set the ID of the entity.
         * @param new_id The new ID for the entity.
         */
        void set_id(EntityID new_id) { id = new_id; }
    };
} // namespace gam300
#endif // GAM300_ENTITY_H