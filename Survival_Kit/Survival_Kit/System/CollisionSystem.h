/******************************************************************************/
/*!
\file       CollisionSystem.h
\author     (you)
\date       Oct 03 2025
\brief      Minimal AABB vs AABB collision detection and resolution.
            - Requires Transform3D + Collider
            - If RigidBody exists, applies positional correction + impulse
            - If no RigidBody, treated as static (invMass = 0)
/******************************************************************************/
#pragma once
#ifndef __COLLISION_SYSTEM_H__
#define __COLLISION_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/Collider.h"
#include "../Component/RigidBody.h"

namespace gam300
{
    class SystemManager;

    class CollisionSystem : public ComponentSystem<Transform3D, Collider>
    {
    public:
        CollisionSystem();

        bool init(SystemManager &system_manager) override;
        void update(float dt) override;
        void shutdown() override;

        // Not used; pairwise resolution happens inside update()
        void process_entity(EntityID /*entity_id*/) override {}

        // Tunables
        void setRestitution(float e) { m_restitution = e; } // 0 inelastic, 1 bouncy
        void setPositionalCorrection(float percent, float slop)
        {
            m_posCorrectPercent = percent; m_posSlop = slop;
        }

    private:
        float m_dt{ 0.0f };
        float m_restitution{ 0.0f };        // coefficient of restitution
        float m_posCorrectPercent{ 0.8f };  // Baumgarte positional correction factor
        float m_posSlop{ 0.01f };           // penetration slop

        struct Contact
        {
            EntityID a, b;
            Vector3D normal;     // from A -> B along min-penetration axis
            float    penetration;
        };

        static bool computeAABBCollision(const Transform3D &trA, const Collider &colA,
            const Transform3D &trB, const Collider &colB,
            Vector3D &outNormal, float &outPenetration);

        static inline float signf(float x) { return (x >= 0.0f) ? 1.0f : -1.0f; }

        void resolve(Contact &c);
        void positionalCorrection(const Contact &c, Transform3D *tA, RigidBody *rbA,
            Transform3D *tB, RigidBody *rbB);
        void applyImpulse(const Contact &c, RigidBody *rbA, RigidBody *rbB);
    };
}

#endif // __COLLISION_SYSTEM_H__
