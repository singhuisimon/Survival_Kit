/**
 * @file AIComponent.cpp
 * @brief Implementation of the AIComponent class.
 * @details Provides the implementation for AI component initialization and updates.
 * @author Simon Chan
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "AIComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    AIComponent::AIComponent()
        : m_is_active(true) {
    }

    void AIComponent::init(EntityID entity_id) {
        m_owner_id = entity_id;
        LM.writeLog("AIComponent::init() - Initialized for entity %u", entity_id);
    }

    void AIComponent::update(float dt) {
        // Base update - override or extend in derived classes
        // AI logic will be handled primarily by AISystem

        // Unreferenced parameter
        (void)dt;
    }

} // namespace gam300