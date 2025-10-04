/******************************************************************************/
/*!
\file       PhysicsSystem.h
\author     Low Yue Jun (yuejun.low)
\date       Oct 03 2025
\brief      Physics system that updates RigidBody each frame.
           - Finds (Transform3D, RigidBody) pairs
           - Calls integrateForces / integrateVelocity with stored dt
*/
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

        bool init(SystemManager& sysMgr) override;
        void update(float dt) override;
        void shutdown() override;

        // NOTE: matches System base signature (no dt here)
        void process_entity(EntityID entity_id) override;

    private:
        ECSManager& PhysicsEcsRef; // bound to EM in ctor
        float       m_dt{ 0.0f };
    };
} // namespace gam300

#endif // __PHYSICS_SYSTEM_H__
