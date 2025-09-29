/******************************************************************************/
/*!
\file       MovementSystem.h
\author     (you)
\date       Oct 03 2025
\brief      Input-driven movement wired to new RigidBodyComponent.
            - If mass == 0 (invMass == 0) -> treat as kinematic: move Transform directly.
            - Else dynamic: convert input into per-frame forces via ForceManager.
/******************************************************************************/
#pragma once
#ifndef __MOVEMENT_SYSTEM_H__
#define __MOVEMENT_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"

namespace gam300
{
    class SystemManager;

    class MovementSystem : public ComponentSystem<Transform3D, RigidBody>
    {
    public:
        MovementSystem();

        bool init(SystemManager &system_manager) override;
        void update(float dt) override;
        void shutdown() override;

        void process_entity(EntityID entity_id) override;

    private:
        float m_dt{ 0.0f };
        // Force magnitude for dynamic bodies when keys are held (N).
        float m_moveForce{ 20.0f };
        // Bit mask used to tag input forces on the per-entity ForceManager.
        static constexpr unsigned INPUT_FORCE_MASK = 1u << 0;
    };
}

#endif // __MOVEMENT_SYSTEM_H__
