/**
 * @file MovementSystem.h
 * @brief .
 * @details .
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __TRANSFORM_SYSTEM_H__
#define __TRANSFORM_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"

namespace gam300 {

    class MovementSystem : public ComponentSystem<Transform3D, RigidBody>{

    public:
        /**
         * @brief Constructor for...
         */
        MovementSystem();

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

    private:
        float m_dt = 0; 
    };


}


#endif // !__TRANSFORM_SYSTEM_H__
