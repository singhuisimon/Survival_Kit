/**
 * @file AISystem.h
 * @brief Declaration of the AISystem for processing AI-controlled entities.
 * @details Manages AI behavior updates and decision-making for entities with AIComponent.
 * @author
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __AI_SYSTEM_H__
#define __AI_SYSTEM_H__

#include "../System/System.h"
#include "../Component/AIComponent.h"
#include "../Component/Transform3D.h"

namespace gam300 {

    /**
     * @brief System for processing entities with AI components.
     * @details Handles AI decision-making, pathfinding, and behavior updates.
     */
    class AISystem : public ComponentSystem<Transform3D, AIComponent> {
    public:
        /**
         * @brief Constructor for AISystem.
         */
        AISystem();

        /**
         * @brief Destructor for AISystem.
         */
        ~AISystem();

        /**
         * @brief Initialize the AI system.
         * @param system_manager Reference to the system manager.
         * @return True if initialization was successful, false otherwise.
         */
        bool init(SystemManager& system_manager) override;

        /**
         * @brief Update all AI entities.
         * @param dt Delta time since the last update.
         */
        void update(float dt) override;

        /**
         * @brief Shut down the AI system.
         */
        void shutdown() override;

        /**
         * @brief Process a specific AI entity.
         * @param entity_id The ID of the entity to process.
         */
        void process_entity(EntityID entity_id) override;

    private:
        float m_dt;  ///< Cached delta time for current frame
    };

} // namespace gam300

#endif // __AI_SYSTEM_H__