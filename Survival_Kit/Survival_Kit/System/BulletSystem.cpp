/**
 * @file BulletSystem.cpp
 * @brief Implementation of the Bullet System for handling bullet behavior.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../System/BulletSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ECSManager.h"

namespace gam300 {

    BulletSystem::BulletSystem()
        : ComponentSystem<Transform3D, Bullet>("BulletSystem") {
        // Set priority (higher number = updates later)
        // Bullets should update after input/physics but before rendering
        set_priority(75);
    }

    bool BulletSystem::init(SystemManager& system_manager) {
        // Any initialization logic for the bullet system
        (void)system_manager; // Unused parameter

        LM.writeLog("BulletSystem::init() - Bullet System Initialized");
        return true;
    }

    void BulletSystem::update(float dt) {
        m_dt = dt;

        // Process all entities with Transform3D and Bullet components
        for (EntityID entity_id : m_entities) {
            process_entity(entity_id);
        }
    }

    void BulletSystem::shutdown() {
        LM.writeLog("BulletSystem::shutdown() - Bullet System shut down");
    }

    void BulletSystem::process_entity(EntityID entity_id) {
        // Get the required components
        auto transform = CM.get_component<Transform3D>(entity_id);
        auto bullet = CM.get_component<Bullet>(entity_id);

        // Safety check
        if (!transform || !bullet) {
            return;
        }

        // Skip inactive bullets
        if (!bullet->isActive()) {
            // Could destroy the entity here or let a cleanup system handle it
            // EM.destroyEntity(entity_id);
            return;
        }

        // Update bullet movement
        updateBulletMovement(transform, bullet);

        // Check if bullet has expired and deactivate if needed
        if (bullet->hasExpired()) {
            bullet->deactivate();
            LM.writeLog("BulletSystem::process_entity() - Bullet %d expired, destroying entity",
                entity_id);

            // Destroy the bullet entity
            EM.destroyEntity(entity_id);
        }
    }

    void BulletSystem::updateBulletMovement(Transform3D* transform, Bullet* bullet) {
        // Calculate movement delta based on direction and speed
        Vector3D movement = bullet->getDirection() * bullet->getSpeed() * m_dt;

        // Update the transform position
        transform->setPosition(transform->getPosition() + movement);

        // Note: If you're using RigidBody, you might want to use physics forces instead:
        // auto rigidBody = CM.get_component<RigidBody>(entity_id);
        // if (rigidBody) {
        //     Vector3D velocity = bullet->getDirection() * bullet->getSpeed();
        //     rigidBody->setLinearVelocity(velocity);
        // }
    }

} // namespace gam300