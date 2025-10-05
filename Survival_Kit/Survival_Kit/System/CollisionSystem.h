/*****************************************************************************/
/*!
\file       CollisionSystem.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Public interface for the collision system.
            Provides:
            - Sweep-and-Prune broadphase (AABB proxies)
            - SAT(OBB) narrowphase from Transform basis + scale
            - Contact generation, impulse resolution, positional correction

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
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

    /**************************************************************************
     * @brief
     * Collision system implementing broadphase + narrowphase and resolution.
     *
     * @details
     * Entities are filtered in broadphase with AABBs (Sweep-and-Prune).
     * Narrowphase uses SAT on OBBs derived from Transform bases and scales.
     * Resolution applies impulses and optional positional correction.
     **************************************************************************/
    class CollisionSystem final : public ComponentSystem<Transform3D, Collider> {
    public:
        /**************************************************************************
         * @brief
         * Constructs the collision system with default tunables.
         **************************************************************************/
        CollisionSystem();

        /**************************************************************************
         * @brief
         * Initializes internal state and caches any required references.
         *
         * @param system_manager
         * Reference to the SystemManager.
         *
         * @return
         * True if initialization succeeded; otherwise false.
         **************************************************************************/
        bool init(SystemManager &system_manager) override;

        /**************************************************************************
         * @brief
         * Advances the collision pipeline for the frame.
         *
         * @param dt
         * Delta time in seconds.
         **************************************************************************/
        void update(float dt) override;

        /**************************************************************************
         * @brief
         * Releases any resources owned by the system.
         **************************************************************************/
        void shutdown() override;

        /**************************************************************************
         * @brief
         * ComponentSystem hook (unused here; broadphase handles batching).
         *
         * @param entity_id
         * The entity being processed (unused).
         **************************************************************************/
        void process_entity(EntityID /*entity_id*/) override {}

        /**************************************************************************
         * @brief
         * Sets coefficient of restitution used for impulse resolution.
         *
         * @param e
         * Restitution in [0, 1]. 0 = inelastic, 1 = perfectly elastic.
         **************************************************************************/
        void setRestitution(float e) {
            m_restitution = e;
        }

        /**************************************************************************
         * @brief
         * Sets positional correction parameters (Baumgarte-like).
         *
         * @param percent
         * Fraction of penetration to correct each frame (typ. 0.2–0.8).
         * @param slop
         * Penetration slop tolerated before correction (meters).
         **************************************************************************/
        void setPositionalCorrection(float percent, float slop) {
            m_posCorrectPercent = percent;
            m_posSlop = slop;
        }

    private:
        // ---------------- Types ----------------
        /**************************************************************************
         * @brief
         * Narrowphase contact information.
         **************************************************************************/
        struct Contact {
            EntityID a{};
            EntityID b{};
            Vector3D normal{};   // Unit normal from A -> B
            float    penetration{};
        };

        /**************************************************************************
         * @brief
         * Axis-aligned bounding box in world space.
         **************************************************************************/
        struct AABB {
            Vector3D center{};   // world center
            Vector3D half{};     // half-extents (>= 0)
        };

        /**************************************************************************
         * @brief
         * Oriented bounding box (center + ONB axes + half-extents).
         **************************************************************************/
        struct OBB {
            Vector3D center{};   // world center
            Vector3D axis[3]     // orthonormal world axes (x,y,z)
            {
                Vector3D::ZERO,
                Vector3D::ZERO,
                Vector3D::ZERO
            };
            Vector3D half{};     // half-extents (>= 0)
        };

        /**************************************************************************
         * @brief
         * Broadphase proxy for sweep axis.
         **************************************************************************/
        struct Proxy {
            EntityID id{};
            AABB     box{};
            float    minX{};
            float    maxX{};
        };

        // -------------- Config --------------
        float m_dt{ 0.0f };
        float m_restitution{ 0.0f };        // 0 inelastic, 1 bouncy
        float m_posCorrectPercent{ 0.8f };  // Baumgarte factor
        float m_posSlop{ 0.01f };           // penetration slop

        // -------------- Constants --------------
        // Keep constants local to the class (header-safe).
        static constexpr float cEpsilon{ 1e-5f };

        // -------------- Small utils --------------
        /**************************************************************************
         * @brief
         * Returns +1 for x >= 0, else -1.
         **************************************************************************/
        static inline float signf(float x) {
            return (x >= 0.0f) ? 1.0f : -1.0f;
        }

        /**************************************************************************
         * @brief
         * Clamps v to be at least mn.
         **************************************************************************/
        static inline float clampMin(float v, float mn) {
            return (v < mn) ? mn : v;
        }

        // -------- Vector math ----------
        /**************************************************************************
         * @brief Dot product of two vectors. *************************************************/
        static inline float dot(Vector3D const &a, Vector3D const &b) {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        /**************************************************************************
         * @brief Cross product of two vectors. *************************************************/
        static inline Vector3D cross(Vector3D const &a, Vector3D const &b) {
            return Vector3D
            {
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            };
        }

        /**************************************************************************
         * @brief Vector addition. *************************************************/
        static inline Vector3D add(Vector3D const &a, Vector3D const &b) {
            return Vector3D{ a.x + b.x, a.y + b.y, a.z + b.z };
        }

        /**************************************************************************
         * @brief Vector subtraction. *************************************************/
        static inline Vector3D sub(Vector3D const &a, Vector3D const &b) {
            return Vector3D{ a.x - b.x, a.y - b.y, a.z - b.z };
        }

        /**************************************************************************
         * @brief Scalar multiply. *************************************************/
        static inline Vector3D mul(Vector3D const &a, float s) {
            return Vector3D{ a.x * s, a.y * s, a.z * s };
        }

        // -------------- Broadphase --------------
        /**************************************************************************
         * @brief
         * Builds a world-space AABB for the given entity from Transform/Collider.
         *
         * @param e
         * Entity ID.
         *
         * @return
         * World-space AABB.
         **************************************************************************/
        static AABB buildWorldAABB(EntityID e);

        /**************************************************************************
         * @brief
         * 1D overlap test used by Sweep-and-Prune.
         **************************************************************************/
        static inline bool aabbOverlap1D(float aMin, float aMax, float bMin, float bMax) {
            return !(aMax < bMin || bMax < aMin);
        }

        /**************************************************************************
         * @brief
         * 3D AABB overlap (XYZ must overlap).
         **************************************************************************/
        static inline bool aabbOverlap3D(AABB const &A, AABB const &B) {
            return aabbOverlap1D(A.center.x - A.half.x, A.center.x + A.half.x,
                                 B.center.x - B.half.x, B.center.x + B.half.x)
                && aabbOverlap1D(A.center.y - A.half.y, A.center.y + A.half.y,
                                 B.center.y - B.half.y, B.center.y + B.half.y)
                && aabbOverlap1D(A.center.z - A.half.z, A.center.z + A.half.z,
                                 B.center.z - B.half.z, B.center.z + B.half.z);
        }

        /**************************************************************************
         * @brief
         * Optional simple AABB vs AABB contact (fallback / debugging).
         *
         * @param trA, colA
         * Transform/Collider of A.
         * @param trB, colB
         * Transform/Collider of B.
         * @param outNormal
         * Contact normal from A to B (unit).
         * @param outPenetration
         * Penetration depth (>= 0).
         *
         * @return
         * True if overlapping; normal/penetration are valid on success.
         **************************************************************************/
        static bool computeAABBCollision(Transform3D const &trA, Collider const &colA,
                                         Transform3D const &trB, Collider const &colB,
                                         Vector3D &outNormal, float &outPenetration);

        // -------------- Narrowphase (OBB) --------------
        /**************************************************************************
         * @brief
         * Extracts world-space orthonormal basis vectors from a Transform.
         *
         * @param tr
         * Transform3D providing orientation/scale.
         * @param x,y,z
         * Outputs: unit basis axes.
         **************************************************************************/
        static void extractBasis(Transform3D const &tr, Vector3D &x, Vector3D &y, Vector3D &z);

        /**************************************************************************
         * @brief
         * Builds a world-space OBB from an entity's Transform/Collider.
         **************************************************************************/
        static OBB buildWorldOBB(EntityID e);

        /**************************************************************************
         * @brief
         * SAT test for OBB vs OBB. Returns separating axis data on hit.
         *
         * @param A, B
         * World-space OBBs.
         * @param outNormal
         * Minimum-penetration axis from A -> B (unit).
         * @param outPenetration
         * Minimum penetration depth along outNormal.
         *
         * @return
         * True if overlapping; normal/penetration are valid on success.
         **************************************************************************/
        static bool computeOBBOBBCollision(OBB const &A, OBB const &B,
                                           Vector3D &outNormal, float &outPenetration);

        // -------------- Resolution --------------
        /**************************************************************************
         * @brief
         * Resolves a single contact by applying impulses and correction.
         **************************************************************************/
        void resolve(Contact &c);

        /**************************************************************************
         * @brief
         * Positional correction to reduce lingering penetrations.
         *
         * @param c
         * Contact information (normal, depth).
         * @param tA, rbA
         * Transform/RigidBody of A (nullable if static).
         * @param tB, rbB
         * Transform/RigidBody of B (nullable if static).
         **************************************************************************/
        void positionalCorrection(Contact const &c,
                                  Transform3D *tA, RigidBody *rbA,
                                  Transform3D *tB, RigidBody *rbB);

        /**************************************************************************
         * @brief
         * Applies collision impulse along the contact normal.
         *
         * @param c
         * Contact to resolve.
         * @param rbA, rbB
         * Rigid bodies for the two entities (nullable if static/kinematic).
         **************************************************************************/
        void applyImpulse(Contact const &c, RigidBody *rbA, RigidBody *rbB);
    };
} // namespace gam300

#endif // __COLLISION_SYSTEM_H__
