/**
 * @file BulletSystem.h
 * @brief Declaration of the Bullet System for handling bullet/projectile behavior.
 * @details Processes entities with Bullet components for movement and collision.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __BULLET_SYSTEM_H__
#define __BULLET_SYSTEM_H__

#include "../Component/Transform3D.h"
#include "../Component/Bullet.h"
#include "../Component/RigidBody.h"
#include "../System/System.h"

namespace gam300 {

    /**
     * @brief System for processing bullet entities.
     * @details Handles bullet movement, lifetime tracking, and cleanup.
     */
    class BulletSystem : public ComponentSystem<Transform3D, Bullet> {
    public:
        /**
         * @brief Constructor for BulletSystem.
         */
        BulletSystem();

        /**
         * @brief Initialize the system.
         * @param system_manager Reference to the system manager.
         * @return True if initialization was successful, false otherwise.
         */
        bool init(SystemManager& system_manager) override;

        /**
         * @brief Update the system, processing all bullet entities.
         * @param dt Delta time since the last update.
         */
        void update(float dt) override;

        /**
         * @brief Clean up the system when shutting down.
         */
        void shutdown() override;

        /**
         * @brief Process a specific entity with Bullet and Transform3D components.
         * @param entity_id The ID of the entity to process.
         */
        void process_entity(EntityID entity_id) override;

    private:
        float m_dt = 0.0f;  // Delta time for current frame

        /**
         * @brief Update bullet movement based on its direction and speed.
         * @param transform Pointer to the entity's Transform3D component.
         * @param bullet Pointer to the entity's Bullet component.
         */
        void updateBulletMovement(Transform3D* transform, Bullet* bullet);
    };

} // namespace gam300

#endif // __BULLET_SYSTEM_H__