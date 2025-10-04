/*****************************************************************************/
/*!
\file       CollisionSystem.h
\author     Low Yue Jun (yuejun.low)
\date       Oct 03 2025
\brief      Broadphase (Sweep-and-Prune) + Narrowphase:
            - SAT for OBB vs OBB (from Transform basis + scale)
            - Contact/impulse resolution
*/
/*****************************************************************************/
#pragma once
#ifndef __COLLISION_SYSTEM_H__
#define __COLLISION_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/Collider.h"
#include "../Component/RigidBody.h"

#include <vector>
#include <algorithm>

namespace gam300 {
    class SystemManager;

    class CollisionSystem : public ComponentSystem<Transform3D, Collider> {
    public:
        CollisionSystem();

        bool init(SystemManager &system_manager) override;
        void update(float dt) override;
        void shutdown() override;

        void process_entity(EntityID /*entity_id*/) override {}

        // Tunables
        void setRestitution(float e) {
            m_restitution = e;
        } // [0..1]
        void setPositionalCorrection(float percent, float slop) {
            m_posCorrectPercent = percent; m_posSlop = slop;
        }

    private:
        // ---------------- Data ----------------
        struct Contact {
            EntityID a, b;
            Vector3D normal;     // from A -> B (unit)
            float    penetration;
        };

        struct AABB {
            Vector3D center;
            Vector3D half;       // >= 0
        };

        // Oriented Box (center + orthonormal basis axes + half extents)
        struct OBB {
            Vector3D center;
            Vector3D axis[3];    // normalized world axes x,y,z
            Vector3D half;       // >= 0
        };

        struct Proxy {
            EntityID id;
            AABB     box;
            float    minX, maxX;
        };

        // -------------- Config --------------
        float m_dt{ 0.0f };
        float m_restitution{ 0.0f };        // 0 inelastic, 1 bouncy
        float m_posCorrectPercent{ 0.8f };  // Baumgarte factor
        float m_posSlop{ 0.01f };           // penetration slop

        // -------------- Utils --------------
        static inline float signf(float x) {
            return (x >= 0.0f) ? 1.0f : -1.0f;
        }
        static inline float clampMin(float v, float mn) {
            return (v < mn) ? mn : v;
        }

        // Broadphase helpers
        static AABB buildWorldAABB(EntityID e);
        static inline bool aabbOverlap1D(float aMin, float aMax, float bMin, float bMax) {
            return !(aMax < bMin || bMax < aMin);
        }
        static inline bool aabbOverlap3D(const AABB &A, const AABB &B) {
            return aabbOverlap1D(A.center.x - A.half.x, A.center.x + A.half.x, B.center.x - B.half.x, B.center.x + B.half.x) &&
                aabbOverlap1D(A.center.y - A.half.y, A.center.y + A.half.y, B.center.y - B.half.y, B.center.y + B.half.y) &&
                aabbOverlap1D(A.center.z - A.half.z, A.center.z + A.half.z, B.center.z - B.half.z, B.center.z + B.half.z);
        }

        // AABB contact (kept as optional fallback if you want)
        static bool computeAABBCollision(const Transform3D &trA, const Collider &colA,
                                         const Transform3D &trB, const Collider &colB,
                                         Vector3D &outNormal, float &outPenetration);

        // Resolution
        void resolve(Contact &c);
        void positionalCorrection(const Contact &c, Transform3D *tA, RigidBody *rbA,
                                  Transform3D *tB, RigidBody *rbB);
        void applyImpulse(const Contact &c, RigidBody *rbA, RigidBody *rbB);

        // -------- Basis / OBB ----------
        static void extractBasis(const Transform3D &tr, Vector3D &x, Vector3D &y, Vector3D &z);
        static OBB  buildWorldOBB(EntityID e);

        // -------- Vector math ----------
        static inline float dot(const Vector3D &a, const Vector3D &b) {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }
        static inline Vector3D cross(const Vector3D &a, const Vector3D &b) {
            return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
        }
        static inline Vector3D add(const Vector3D &a, const Vector3D &b) {
            return { a.x + b.x, a.y + b.y, a.z + b.z };
        }
        static inline Vector3D sub(const Vector3D &a, const Vector3D &b) {
            return { a.x - b.x, a.y - b.y, a.z - b.z };
        }
        static inline Vector3D mul(const Vector3D &a, float s) {
            return { a.x * s, a.y * s, a.z * s };
        }

        // -------- SAT: OBB vs OBB ----------
        static bool computeOBBOBBCollision(const OBB &A, const OBB &B,
                                           Vector3D &outNormal, float &outPenetration);
    };
}

#endif // __COLLISION_SYSTEM_H__
