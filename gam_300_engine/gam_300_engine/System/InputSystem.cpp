/**
 * @file InputSystem.cpp
 * @brief Implementation of the Input System for the Entity Component System.
 * @details Contains implementations for all member functions declared in InputSystem.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../System/InputSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    // Constructor
    InputSystem::InputSystem() : ComponentSystem<InputComponent>("InputSystem") {
        // Set priority - input system should run early in the update cycle
        set_priority(100);
    }

    // Initialize the system
    bool InputSystem::init(SystemManager& /*system_manager*/) {
        LM.writeLog("InputSystem::init() - Input System initialized");
        return true;
    }

    // Update the system
    void InputSystem::update(float dt) {
        // Mark parameter as unused to avoid compiler warning
        (void)dt;  // Explicitly tell compiler that this parameter is intentionally unused

        // Process all entities that have InputComponents
        for (EntityID entity_id : m_entities) {
            process_entity(entity_id);
        }
    }

    // Shut down the system
    void InputSystem::shutdown() {
        LM.writeLog("InputSystem::shutdown() - Input System shut down");
    }

    // Process a specific entity
    void InputSystem::process_entity(EntityID entity_id) {
        // Get the InputComponent for this entity
        InputComponent* input_component = CM.get_component<InputComponent>(entity_id);

        // Make sure we have a valid component
        if (input_component && input_component->isActive()) {
            // Update the input component (which will process all mapped actions)
            input_component->update(0.0f); // dt not used in input processing
        }
    }

} // namespace gam300