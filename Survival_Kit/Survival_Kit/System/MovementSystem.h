/******************************************************************************/
/*!
\file       MovementSystem.h
\author     (you)
\date       Oct 03 2025
\brief      Input-driven movement that applies forces to RigidBody.
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

        // Force magnitude (Newtons) applied while a key is held (for dynamic bodies)
        float m_moveForce{ 40.0f };

        // Direct-move speed for static bodies (units/sec)
        float m_kinematicSpeed{ 2.0f };

        // Mask to tag per-frame input forces in ForceManager
        static constexpr unsigned INPUT_FORCE_MASK = 1u << 0;
    };
}

#endif // __MOVEMENT_SYSTEM_H__
