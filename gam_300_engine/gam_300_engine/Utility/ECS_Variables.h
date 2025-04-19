#pragma once
/**
 * @file ECS_Variables.h
 * @brief Common Variables definitions for the Entity Component System (ECS).
 * @details Provides Variables aliases and constants used throughout the ECS implementation.
 * @author Simon Chan (100%)
 * @date September 15, 2024
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef GAM300_TYPE_H
#define GAM300_TYPE_H

#include <cstdint>
#include <bitset>
#include <vector>
#include <unordered_map>

namespace gam300 {

    /**
     * @brief Maximum number of different component types allowed in the system.
     * @details This determines the size of the ComponentMask bitset and limits how many
     *          different component types can be registered in the ECS.
     */
    constexpr std::size_t MAX_COMPONENTS = 64;

    /**
     * @brief Type used for entity identifiers.
     * @details Using a 32-bit unsigned integer allows for up to ~4 billion unique entities,
     *          which should be more than sufficient for most game applications.
     */
    using EntityID = std::uint32_t;

    /**
     * @brief Invalid entity ID constant.
     * @details Used to represent a null or invalid entity reference.
     */
    constexpr EntityID INVALID_ENTITY_ID = 0;

    /**
     * @brief Bitset that represents which components an entity has.
     * @details Each bit corresponds to a component type ID. If the bit is set,
     *          the entity has that component.
     */
    using ComponentMask = std::bitset<MAX_COMPONENTS>;

    /**
     * @brief Type used for component type identifiers.
     * @details Each component type gets a unique ID that is used in the component mask.
     */
    using ComponentTypeID = std::size_t;

    /**
     * @brief Type used for system identifiers.
     * @details Each system in the ECS gets a unique identifier.
     */
    using SystemID = std::size_t;

    /**
     * @brief Invalid component type ID constant.
     * @details Used to represent a null or invalid component type reference.
     */
    constexpr ComponentTypeID INVALID_COMPONENT_ID = static_cast<ComponentTypeID>(-1);

    /**
     * @brief Collection of entity IDs.
     * @details Commonly used for systems to track which entities they process.
     */
    using EntityVector = std::vector<EntityID>;

    /**
     * @brief Map from entity ID to a value.
     * @details Template for various mappings from entity ID to different value types.
     */
    template<typename T>
    using EntityMap = std::unordered_map<EntityID, T>;

} // namespace GAM300

#endif // GAM300_TYPE_H