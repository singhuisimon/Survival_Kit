/******************************************************************************/
/*!
\file       MovementController.h
\author     Simon Chan
\date       Oct 04 2025
\brief      Component for configurable movement behavior that can be driven
            by scripts or direct input. Replaces hard-coded movement logic.
/******************************************************************************/
#pragma once
#ifndef __MOVEMENT_CONTROLLER_H__
#define __MOVEMENT_CONTROLLER_H__

#include "../Component/Component.h"
#include "../Utility/Vector3D.h"

namespace gam300
{
    /**
     * @brief Movement modes for entities
     */
    enum class MovementMode
    {
        SCRIPT_CONTROLLED,  // Movement controlled by attached scripts
        DIRECT_INPUT,       // Movement controlled by direct input (legacy mode)
        DISABLED            // No movement
    };

    /**
     * @brief Component for scriptable movement control
     * @details Stores movement configuration and desired movement vectors
     *          that can be set by scripts or input systems
     */
    class MovementController : public Component
    {
    public:
        MovementController(
            MovementMode mode = MovementMode::SCRIPT_CONTROLLED,
            float moveForce = 40.0f,
            float kinematicSpeed = 2.0f
        );

        void init(EntityID entity_id) override;
        void update(float dt) override;

        // Movement mode
        MovementMode getMovementMode() const { return m_mode; }
        void setMovementMode(MovementMode mode) { m_mode = mode; }

        // Force/speed settings
        float getMoveForce() const { return m_moveForce; }
        void setMoveForce(float force) { m_moveForce = force; }

        float getKinematicSpeed() const { return m_kinematicSpeed; }
        void setKinematicSpeed(float speed) { m_kinematicSpeed = speed; }

        // Desired movement direction (set by scripts)
        const Vector3D& getDesiredDirection() const { return m_desiredDirection; }
        void setDesiredDirection(const Vector3D& dir) { m_desiredDirection = dir; }

        // Desired rotation (set by scripts)
        const Vector3D& getDesiredRotation() const { return m_desiredRotation; }
        void setDesiredRotation(const Vector3D& rot) { m_desiredRotation = rot; }

        // Torque magnitude for rotation
        float getRotationTorque() const { return m_rotationTorque; }
        void setRotationTorque(float torque) { m_rotationTorque = torque; }

        // Clear movement inputs (typically called each frame before scripts run)
        void clearInputs()
        {
            m_desiredDirection = Vector3D(0.0f, 0.0f, 0.0f);
            m_desiredRotation = Vector3D(0.0f, 0.0f, 0.0f);
        }

    private:
        MovementMode m_mode;

        // Movement parameters
        float m_moveForce;          // Force magnitude for dynamic bodies (Newtons)
        float m_kinematicSpeed;     // Speed for kinematic bodies (units/sec)
        float m_rotationTorque;     // Torque magnitude for rotation (default: 20.0f)

        // Desired movement inputs (set by scripts or input)
        Vector3D m_desiredDirection;    // Desired movement direction
        Vector3D m_desiredRotation;     // Desired rotation axis/direction
    };
}

#endif // __MOVEMENT_CONTROLLER_H__