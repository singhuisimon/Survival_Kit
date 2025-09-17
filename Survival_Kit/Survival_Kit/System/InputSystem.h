/**
 * @file InputSystem.h
 * @brief Declaration of the Input System for the Entity Component System.
 * @details Processes all entities with InputComponents.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __INPUT_SYSTEM_H__
#define __INPUT_SYSTEM_H__

#include "../System/System.h"
#include "../Component/InputComponent.h"

namespace gam300 {

    /**
     * @brief System for processing entity input components.
     * @details Updates all entities with InputComponents, processing their input mappings.
     */
    class InputSystem : public ComponentSystem<InputComponent> {
    public:
        /**
         * @brief Constructor for InputSystem.
         */
        InputSystem();

        /**
         * @brief Initialize the system.
         * @param system_manager Reference to the system manager.
         * @return True if initialization was successful, false otherwise.
         */
        bool init(SystemManager& system_manager) override;

        /**
         * @brief Update the system, processing all relevant entities.
         * @param dt Delta time since the last update.
         */
        void update(float dt) override;

        /**
         * @brief Clean up the system when shutting down.
         */
        void shutdown() override;

        /**
         * @brief Process a specific entity with an InputComponent.
         * @param entity_id The ID of the entity to process.
         */
        void process_entity(EntityID entity_id) override;
    };

} // namespace gam300

#endif // __INPUT_SYSTEM_H__