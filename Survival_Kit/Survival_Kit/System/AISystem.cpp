/**
 * @file AISystem.cpp
 * @brief Implementation of the AISystem for processing AI entities.
 * @details Manages AI behavior updates and decision-making for entities with AIComponent.
 * @author
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "AISystem.h"
#include "../Manager/ECSManager.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    AISystem::AISystem()
        : ComponentSystem("AISystem")
        , m_dt(0.0f) {
    }

    AISystem::~AISystem() {
        shutdown();
    }

    bool AISystem::init(SystemManager& system_manager) {
        LM.writeLog("AISystem::init() - Initializing AI System");

        // Add any initialization logic here
        // For example: loading AI configurations, setting up pathfinding, etc.

        LM.writeLog("AISystem::init() - AI System initialized successfully");

        // Unreferenced parameter
        (void)system_manager;

        return true;
    }

    void AISystem::update(float dt) {
        // Cache delta time
        m_dt = dt;

        // Process all entities with AIComponent
        for (EntityID entity_id : m_entities) {
            process_entity(entity_id);
        }
    }

    void AISystem::shutdown() {
        LM.writeLog("AISystem::shutdown() - Shutting down AI System");

        // Clear entity list
        m_entities.clear();

        // Add any cleanup logic here

        LM.writeLog("AISystem::shutdown() - AI System shut down successfully");
    }

    void AISystem::process_entity(EntityID entity_id) {
        // Get the AI component
        AIComponent* ai_comp = CM.get_component<AIComponent>(entity_id);
        if (!ai_comp || !ai_comp->is_active()) {
            return;
        }

        // Get the transform component
        Transform3D* transform = CM.get_component<Transform3D>(entity_id);
        if (!transform) {
            return;
        }

        // TODO: Add AI logic here
        // Examples:
        // - State machine updates
        // - Pathfinding
        // - Decision making
        // - Behavior tree evaluation
        // - Sensor processing

        // Unreferenced parameter
        (void)m_dt;
    }

} // namespace gam300