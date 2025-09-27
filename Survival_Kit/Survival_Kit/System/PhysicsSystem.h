/**
 * @file ...
 * @brief ...
 * @details ...
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __PHYSICS_SYSTEM_H__
#define __PHYSICS_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

namespace gam300 {

    //IDK how Transform3D also fits into this
    class PhysicsSystem : ComponentSystem<RigidBody> {

    private:
        class ECSManager& PhysicsEcsRef;
    public:
        /**
         * @brief Constructor for PhysicsSystem.
         */
        PhysicsSystem();

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

        //void process_entity(EntityID entity_id) override;

        void process_entity(EntityID entity_id, float dt);
    };
}

#endif // !__PHYSICS_SYSTEM_H__
