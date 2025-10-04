/******************************************************************************/
/*!
\file       MovementSystem.h
\author     (you)
\date       Oct 04 2025
\brief      Refactored movement system that works with scriptable movement
            controllers instead of hard-coded input logic.
/******************************************************************************/
#pragma once
#ifndef __MOVEMENT_SYSTEM_H__
#define __MOVEMENT_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include "../Component/MovementController.h"

namespace gam300
{
    class SystemManager;

    /**
     * @brief System that processes movement based on MovementController components
     * @details Applies movement forces/velocities based on controller settings.
     *          Movement direction can be set by scripts or direct input.
     */
    class MovementSystem : public ComponentSystem<Transform3D, RigidBody, MovementController>
    {
    public:
        MovementSystem();

        bool init(SystemManager& system_manager) override;
        void update(float dt) override;
        void shutdown() override;

        void process_entity(EntityID entity_id) override;

    private:
        float m_dt{ 0.0f };

        // Force masks for input forces/torques
        static constexpr unsigned INPUT_FORCE_MASK = 1u << 0;
        static constexpr unsigned INPUT_TORQUE_MASK = 1u << 1;

        /**
         * @brief Apply movement for static/kinematic bodies (direct position change)
         */
        void applyKinematicMovement(Transform3D* transform, MovementController* controller);

        /**
         * @brief Apply movement for dynamic bodies (force-based)
         */
        void applyDynamicMovement(RigidBody* rigidBody, MovementController* controller);

        /**
         * @brief Apply rotation for dynamic bodies (torque-based)
         */
        void applyDynamicRotation(RigidBody* rigidBody, MovementController* controller);

        /**
         * @brief Legacy direct input handling (for DIRECT_INPUT mode)
         */
        void processDirectInput(EntityID entity_id, Transform3D* transform,
            RigidBody* rigidBody, MovementController* controller);
    };
}

#endif // __MOVEMENT_SYSTEM_H__