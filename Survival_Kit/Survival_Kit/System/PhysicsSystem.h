/******************************************************************************/
/*!
\file       PhysicsSystem.h
\author     (you)
\date       Oct 03 2025
\brief      Physics system that updates RigidBodyComponent each frame.
           - Finds (Transform3D, RigidBodyComponent) pairs
           - Calls rb->Integrate(*transform, dt)
/******************************************************************************/
#pragma once
#ifndef __PHYSICS_SYSTEM_H__
#define __PHYSICS_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"

namespace gam300
{
    class SystemManager;
    class ECSManager;

    // Assumes ComponentSystem<T> is declared in System.h
    class PhysicsSystem : public ComponentSystem<RigidBody>
    {
    public:
        PhysicsSystem();

        bool init(SystemManager &sysMgr) override;
        void update(float dt) override;
        void shutdown() override;

        void process_entity(EntityID entity_id, float dt);

    private:
        ECSManager &PhysicsEcsRef; // bound to EM in ctor
    };
} // namespace gam300

#endif // __PHYSICS_SYSTEM_H__
