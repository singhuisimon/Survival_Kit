/**
 * @file Bullet.cpp
 * @brief Implementation of the Bullet Component for the Entity Component System.
 * @details Contains implementations for all member functions declared in Bullet.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Component/Bullet.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    // Constructor
    Bullet::Bullet(const Vector3D& direction, float speed, float damage,
        float lifetime, EntityID owner_entity)
        : m_direction(direction.normalize()),
        m_speed(speed),
        m_damage(damage),
        m_lifetime(lifetime),
        m_elapsed_time(0.0f),
        m_owner_entity(owner_entity),
        m_is_active(true) {
        // Initialize with provided values
        // Direction is normalized to ensure consistent behavior
    }

    // Initialize the component
    void Bullet::init(EntityID entity_id) {
        m_owner_id = entity_id;
        m_elapsed_time = 0.0f;
        m_is_active = true;

        LM.writeLog("Bullet::init() - Bullet component initialized for entity %d "
            "(Speed: %.2f, Damage: %.2f, Lifetime: %.2f)",
            entity_id, m_speed, m_damage, m_lifetime);
    }

    // Update the component
    void Bullet::update(float dt) {
        if (!m_is_active) {
            return;
        }

        // Update elapsed time
        m_elapsed_time += dt;

        // Check if bullet has expired
        if (hasExpired()) {
            deactivate();
            LM.writeLog("Bullet::update() - Bullet entity %d has expired after %.2f seconds",
                m_owner_id, m_elapsed_time);
        }

        // Note: Actual movement should be handled by a BulletSystem that works with
        // Transform3D and/or RigidBody components. This update just tracks lifetime.
    }

    // Set the bullet's direction (with normalization)
    void Bullet::setDirection(const Vector3D& direction) {
        m_direction = direction.normalize();
    }

} // namespace gam300