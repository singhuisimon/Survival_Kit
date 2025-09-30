/**
 * @file Bullet.h
 * @brief Declaration of the Bullet Component for the Entity Component System.
 * @details Handles bullet behavior including damage, speed, and lifetime.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __BULLET_H__
#define __BULLET_H__

#include "../Component/Component.h"
#include "../Utility/Vector3D.h"

namespace gam300 {

    /**
     * @brief Component for bullet/projectile entities.
     * @details Stores bullet properties like speed, damage, and lifetime.
     */
    class Bullet : public Component {
    private:
        Vector3D m_direction;       // Direction of bullet travel (normalized)
        float m_speed;              // Speed of the bullet (units per second)
        float m_damage;             // Damage dealt on impact
        float m_lifetime;           // Maximum time bullet can exist (seconds)
        float m_elapsed_time;       // Time since bullet was created (seconds)
        EntityID m_owner_entity;    // Entity that fired this bullet
        bool m_is_active;           // Whether the bullet is still active

    public:
        /**
         * @brief Constructor for Bullet component.
         * @param direction Direction of bullet travel (will be normalized)
         * @param speed Speed of the bullet (default: 10.0)
         * @param damage Damage dealt on impact (default: 10.0)
         * @param lifetime Maximum lifetime in seconds (default: 5.0)
         * @param owner_entity Entity that fired this bullet (default: INVALID_ENTITY_ID)
         */
        Bullet(const Vector3D& direction = Vector3D(1.0f, 0.0f, 0.0f),
            float speed = 10.0f,
            float damage = 10.0f,
            float lifetime = 5.0f,
            EntityID owner_entity = INVALID_ENTITY_ID);

        /**
         * @brief Initialize the component after creation.
         * @param entity_id The ID of the entity this component is attached to.
         */
        void init(EntityID entity_id) override;

        /**
         * @brief Update the component state.
         * @param dt Delta time in seconds.
         */
        void update(float dt) override;

        // Getters
        /**
         * @brief Get the bullet's direction.
         * @return Direction vector (normalized).
         */
        const Vector3D& getDirection() const { return m_direction; }

        /**
         * @brief Get the bullet's speed.
         * @return Speed in units per second.
         */
        float getSpeed() const { return m_speed; }

        /**
         * @brief Get the bullet's damage.
         * @return Damage value.
         */
        float getDamage() const { return m_damage; }

        /**
         * @brief Get the bullet's maximum lifetime.
         * @return Lifetime in seconds.
         */
        float getLifetime() const { return m_lifetime; }

        /**
         * @brief Get the elapsed time since bullet creation.
         * @return Elapsed time in seconds.
         */
        float getElapsedTime() const { return m_elapsed_time; }

        /**
         * @brief Get the entity that owns/fired this bullet.
         * @return Owner entity ID.
         */
        EntityID getOwnerEntity() const { return m_owner_entity; }

        /**
         * @brief Check if the bullet is still active.
         * @return True if active, false otherwise.
         */
        bool isActive() const { return m_is_active; }

        /**
         * @brief Check if the bullet has expired (lifetime exceeded).
         * @return True if expired, false otherwise.
         */
        bool hasExpired() const { return m_elapsed_time >= m_lifetime; }

        // Setters
        /**
         * @brief Set the bullet's direction.
         * @param direction New direction (will be normalized).
         */
        void setDirection(const Vector3D& direction);

        /**
         * @brief Set the bullet's speed.
         * @param speed New speed in units per second.
         */
        void setSpeed(float speed) { m_speed = speed; }

        /**
         * @brief Set the bullet's damage.
         * @param damage New damage value.
         */
        void setDamage(float damage) { m_damage = damage; }

        /**
         * @brief Set the bullet's lifetime.
         * @param lifetime New lifetime in seconds.
         */
        void setLifetime(float lifetime) { m_lifetime = lifetime; }

        /**
         * @brief Set the bullet's owner entity.
         * @param owner_entity New owner entity ID.
         */
        void setOwnerEntity(EntityID owner_entity) { m_owner_entity = owner_entity; }

        /**
         * @brief Deactivate the bullet.
         * @details Called when bullet hits something or expires.
         */
        void deactivate() { m_is_active = false; }

        /**
         * @brief Reset the elapsed time counter.
         */
        void resetElapsedTime() { m_elapsed_time = 0.0f; }
    };

} // namespace gam300

#endif // __BULLET_H__