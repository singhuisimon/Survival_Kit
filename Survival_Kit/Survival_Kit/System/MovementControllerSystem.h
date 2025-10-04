/******************************************************************************/
/*!
\file       MovementControllerSystem.h
\author     Simon Chan
\date       Oct 04 2025
\brief      System that processes MovementController components
/******************************************************************************/
#pragma once
#ifndef __MOVEMENT_CONTROLLER_SYSTEM_H__
#define __MOVEMENT_CONTROLLER_SYSTEM_H__

#include "../System/System.h"
#include "../Component/MovementController.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"

namespace gam300
{
    class SystemManager;

    class MovementControllerSystem : public ComponentSystem<MovementController, Transform3D>
    {
    public:
        MovementControllerSystem();

        bool init(SystemManager& system_manager) override;
        void update(float dt) override;
        void shutdown() override;
        void process_entity(EntityID entity_id) override;

    private:
        float m_dt{ 0.0f };

        static constexpr unsigned MOVEMENT_FORCE_MASK = 1u << 2;
        static constexpr unsigned MOVEMENT_TORQUE_MASK = 1u << 3;

        void applyKinematicMovement(
            EntityID entity_id,
            MovementController* controller,
            Transform3D* transform
        );

        void applyDynamicMovement(
            EntityID entity_id,
            MovementController* controller,
            Transform3D* transform,
            RigidBody* rigidBody
        );
    };
}

#endif // __MOVEMENT_CONTROLLER_SYSTEM_H__