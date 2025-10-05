/*****************************************************************************/
/*!
\file       CollisionSystem.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      SAP broadphase + SAT(OBB) narrowphase; AABB/SAT contact & resolution.
            Pipeline:
            - Broadphase: Sweep-and-Prune on X using AABBs
            - Narrowphase: SAT on OBBs from Transform basis + scale
            - Resolution: impulse + positional correction (Baumgarte-like)

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/


#include "../System/CollisionSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

#include <cmath>
#include <cfloat>
#include <limits>
#include <algorithm>
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>
#include <glm-0.9.9.8/glm/gtc/matrix_transform.hpp>

namespace gam300 {
    /**************************************************************************
     * @brief
     * Constructs the collision system and sets scheduling priority.
     **************************************************************************/
    CollisionSystem::CollisionSystem() :
        ComponentSystem<Transform3D, Collider>{ "CollisionSystem" }
    {
        set_priority(102);
    }

    /**************************************************************************
     * @brief
     * Initializes internal state and logs startup.
     *
     * @param system_manager
     * Reference to SystemManager (unused here).
     *
     * @return
     * true if successful; otherwise false.
     **************************************************************************/
    bool CollisionSystem::init(SystemManager &/*system_manager*/) {
        LM.writeLog("CollisionSystem::init() - Collision System Initialized");
        return true;
    }

    /**************************************************************************
     * @brief
     * Shuts down and logs teardown.
     **************************************************************************/
    void CollisionSystem::shutdown() {
        LM.writeLog("CollisionSystem::shutdown() - Collision System shut down");
    }

    /**************************************************************************
     * @brief
     * Runs one frame of broadphase + narrowphase + resolution.
     *
     * @param dt
     * Delta time in seconds.
     **************************************************************************/
    void CollisionSystem::update(float dt) {
        m_dt = dt;

        auto const &ids{ m_entities };
        size_t const n{ ids.size() };
        if(n < 2) {
            return;
        }

        std::vector<Proxy> proxies{};
        proxies.reserve(n);

        for(EntityID e : ids) {
            AABB box{ buildWorldAABB(e) };

            constexpr float minHalf{ 1e-4f };
            box.half.x = clampMin(std::fabs(box.half.x), minHalf);
            box.half.y = clampMin(std::fabs(box.half.y), minHalf);
            box.half.z = clampMin(std::fabs(box.half.z), minHalf);

            Proxy p{
                e,
                box,
                box.center.x - box.half.x,
                box.center.x + box.half.x
            };
            proxies.emplace_back(p);
        }

        std::sort(proxies.begin(), proxies.end(),
                  [](Proxy const &a, Proxy const &b) { return a.minX < b.minX; });

        std::vector<std::pair<EntityID, EntityID>> candidates{};
        candidates.reserve(n * 2);

        for(size_t i{ 0 }; i < proxies.size(); ++i) {
            Proxy const &A{ proxies[i] };
            for(size_t j{ i + 1 }; j < proxies.size(); ++j) {
                Proxy const &B{ proxies[j] };
                if(B.minX > A.maxX) {
                    break;
                }
                if(aabbOverlap3D(A.box, B.box)) {
                    candidates.emplace_back(A.id, B.id);
                }
            }
        }

        for(auto const &[ea, eb] : candidates) {
            OBB const obbA{ buildWorldOBB(ea) };
            OBB const obbB{ buildWorldOBB(eb) };

            Vector3D normal{ 0.0f, 0.0f, 0.0f };
            float penetration{ 0.0f };

            if(!computeOBBOBBCollision(obbA, obbB, normal, penetration)) {
                continue;
            }

            Contact c{ ea, eb, normal, penetration };
            resolve(c);
        }
    }

    /**************************************************************************
     * @brief
     * Builds world-space AABB for entity e. Prefers Collider’s world AABB,
     * otherwise falls back to Transform position + scale.
     *
     * @param e
     * Entity ID.
     *
     * @return
     * World-space AABB.
     **************************************************************************/
    CollisionSystem::AABB CollisionSystem::buildWorldAABB(EntityID e) {
        if(auto tr{ CM.get_component<Transform3D>(e) }) {
            if(auto col{ CM.get_component<Collider>(e) }) {
                Vector3D c{}, he{};
                col->getWorldAABB(*tr, c, he);
                return AABB{ c, he };
            }
        }

        Vector3D center{ 0.0f, 0.0f, 0.0f };
        Vector3D half{ 0.5f, 0.5f, 0.5f };

        if(auto tr{ CM.get_component<Transform3D>(e) }) {
            center = tr->getPosition();
            Vector3D const s{ tr->getScale() };
            half = Vector3D{
                std::max(std::fabs(0.5f * s.x), 1e-4f),
                std::max(std::fabs(0.5f * s.y), 1e-4f),
                std::max(std::fabs(0.5f * s.z), 1e-4f)
            };
        }

        return AABB{ center, half };
    }

    /**************************************************************************
     * @brief
     * Simple AABB vs AABB contact test and axis-of-minimum-penetration normal.
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
     * true if overlapping; outputs valid normal/penetration on success.
     **************************************************************************/
    bool CollisionSystem::computeAABBCollision(Transform3D const &trA, Collider const &colA,
                                               Transform3D const &trB, Collider const &colB,
                                               Vector3D &outNormal, float &outPenetration) {
        Vector3D cA{}, eA{}, cB{}, eB{};
        colA.getWorldAABB(trA, cA, eA);
        colB.getWorldAABB(trB, cB, eB);

        Vector3D const d{ cB.x - cA.x, cB.y - cA.y, cB.z - cA.z };

        float const ox{ (eA.x + eB.x) - std::fabs(d.x) };
        if(ox <= 0.0f) {
            return false;
        }

        float const oy{ (eA.y + eB.y) - std::fabs(d.y) };
        if(oy <= 0.0f) {
            return false;
        }

        float const oz{ (eA.z + eB.z) - std::fabs(d.z) };
        if(oz <= 0.0f) {
            return false;
        }

        outPenetration = ox;
        outNormal = Vector3D{ signf(d.x), 0.0f, 0.0f };

        if(oy < outPenetration) {
            outPenetration = oy;
            outNormal = Vector3D{ 0.0f, signf(d.y), 0.0f };
        }

        if(oz < outPenetration) {
            outPenetration = oz;
            outNormal = Vector3D{ 0.0f, 0.0f, signf(d.z) };
        }

        return true;
    }

    /**************************************************************************
     * @brief
     * Resolves a single contact by positional correction and impulse.
     *
     * @param c
     * Contact data (entities, normal, penetration).
     **************************************************************************/
    void CollisionSystem::resolve(Contact &c) {
        auto tA{ CM.get_component<Transform3D>(c.a) };
        auto tB{ CM.get_component<Transform3D>(c.b) };
        auto rbA{ CM.get_component<RigidBody>(c.a) };
        auto rbB{ CM.get_component<RigidBody>(c.b) };

        if(!tA || !tB) {
            return;
        }

        positionalCorrection(c, tA, rbA, tB, rbB);
        applyImpulse(c, rbA, rbB);
    }

    /**************************************************************************
     * @brief
     * Applies positional correction to reduce lingering penetration.
     *
     * @param c
     * Contact (normal, penetration).
     * @param tA, rbA
     * Transform/RigidBody of A (nullable if static).
     * @param tB, rbB
     * Transform/RigidBody of B (nullable if static).
     **************************************************************************/
    void CollisionSystem::positionalCorrection(Contact const &c,
                                               Transform3D *tA, RigidBody *rbA,
                                               Transform3D *tB, RigidBody *rbB) {
        float const invA{ rbA ? rbA->getInvMass() : 0.0f };
        float const invB{ rbB ? rbB->getInvMass() : 0.0f };
        float const invSum{ invA + invB };
        if(invSum <= 0.0f) {
            return;
        }

        float correctionMag{ (c.penetration - m_posSlop) };
        if(correctionMag < 0.0f) {
            correctionMag = 0.0f;
        }
        correctionMag = (correctionMag / invSum) * m_posCorrectPercent;

        Vector3D const corrA{
            -c.normal.x * correctionMag * invA,
            -c.normal.y * correctionMag * invA,
            -c.normal.z * correctionMag * invA
        };

        Vector3D const corrB{
            c.normal.x * correctionMag * invB,
            c.normal.y * correctionMag * invB,
            c.normal.z * correctionMag * invB
        };

        Vector3D pA{ tA->getPosition() };
        pA.x += corrA.x; pA.y += corrA.y; pA.z += corrA.z;
        tA->setPosition(pA);

        Vector3D pB{ tB->getPosition() };
        pB.x += corrB.x; pB.y += corrB.y; pB.z += corrB.z;
        tB->setPosition(pB);
    }

    /**************************************************************************
     * @brief
     * Applies collision impulse along the contact normal.
     *
     * @param c
     * Contact to resolve.
     * @param rbA, rbB
     * Rigid bodies for the two entities (nullable if static/kinematic).
     **************************************************************************/
    void CollisionSystem::applyImpulse(Contact const &c, RigidBody *rbA, RigidBody *rbB) {
        if(!rbA && !rbB) {
            return;
        }

        float const invA{ rbA ? rbA->getInvMass() : 0.0f };
        float const invB{ rbB ? rbB->getInvMass() : 0.0f };
        float const invSum{ invA + invB };
        if(invSum <= 0.0f) {
            return;
        }

        Vector3D const vA{ rbA ? rbA->getVelocity() : Vector3D{ 0.0f, 0.0f, 0.0f } };
        Vector3D const vB{ rbB ? rbB->getVelocity() : Vector3D{ 0.0f, 0.0f, 0.0f } };

        Vector3D const rv{ vB.x - vA.x, vB.y - vA.y, vB.z - vA.z };
        float const relVelN{ rv.x * c.normal.x + rv.y * c.normal.y + rv.z * c.normal.z };
        if(relVelN > 0.0f) {
            return;
        }

        float const e{ m_restitution };
        float const j{ -(1.0f + e) * relVelN / invSum };

        Vector3D const impulse{ c.normal.x * j, c.normal.y * j, c.normal.z * j };

        if(rbA) {
            Vector3D nv{ rbA->getVelocity() };
            nv.x -= impulse.x * invA;
            nv.y -= impulse.y * invA;
            nv.z -= impulse.z * invA;
            rbA->setVelocity(nv);
        }

        if(rbB) {
            Vector3D nv{ rbB->getVelocity() };
            nv.x += impulse.x * invB;
            nv.y += impulse.y * invB;
            nv.z += impulse.z * invB;
            rbB->setVelocity(nv);
        }
    }

    /**************************************************************************
     * @brief
     * Extracts world-space ONB (x,y,z) from Transform’s Euler rotation.
     * Rotation is constructed as q = qz * qy * qx to match Transform3D.
     *
     * @param tr
     * Source transform (rotation).
     * @param x, y, z
     * Outputs: unit basis axes.
     **************************************************************************/
    void CollisionSystem::extractBasis(Transform3D const &tr,
                                       Vector3D &x, Vector3D &y, Vector3D &z) {
        Vector3D const e{ tr.getRotation() };
        glm::quat const qx{ glm::angleAxis(glm::radians(e.x), glm::vec3{ 1.0f, 0.0f, 0.0f }) };
        glm::quat const qy{ glm::angleAxis(glm::radians(e.y), glm::vec3{ 0.0f, 1.0f, 0.0f }) };
        glm::quat const qz{ glm::angleAxis(glm::radians(e.z), glm::vec3{ 0.0f, 0.0f, 1.0f }) };
        glm::quat const q{ qz * qy * qx };

        glm::mat3 const R{ glm::mat3_cast(q) };

        glm::vec3 const Rx{ glm::vec3{ R[0][0], R[1][0], R[2][0] } };
        glm::vec3 const Ry{ glm::vec3{ R[0][1], R[1][1], R[2][1] } };
        glm::vec3 const Rz{ glm::vec3{ R[0][2], R[1][2], R[2][2] } };

        x = Vector3D{ Rx.x, Rx.y, Rx.z }.normalize();
        y = Vector3D{ Ry.x, Ry.y, Ry.z }.normalize();
        z = Vector3D{ Rz.x, Rz.y, Rz.z }.normalize();
    }

    /**************************************************************************
     * @brief
     * Builds a world-space OBB from entity’s Transform (and Collider scale).
     *
     * @param e
     * Entity ID.
     *
     * @return
     * OBB in world space.
     **************************************************************************/
    CollisionSystem::OBB CollisionSystem::buildWorldOBB(EntityID e) {
        OBB b{};
        b.center = Vector3D{ 0.0f, 0.0f, 0.0f };
        b.axis[0] = Vector3D{ 1.0f, 0.0f, 0.0f };
        b.axis[1] = Vector3D{ 0.0f, 1.0f, 0.0f };
        b.axis[2] = Vector3D{ 0.0f, 0.0f, 1.0f };
        b.half = Vector3D{ 0.5f, 0.5f, 0.5f };

        if(auto tr{ CM.get_component<Transform3D>(e) }) {
            b.center = tr->getPosition();
            extractBasis(*tr, b.axis[0], b.axis[1], b.axis[2]);

            Vector3D const s{ tr->getScale() };
            constexpr float eps{ 1e-4f };
            b.half = Vector3D{
                std::max(std::fabs(0.5f * s.x), eps),
                std::max(std::fabs(0.5f * s.y), eps),
                std::max(std::fabs(0.5f * s.z), eps)
            };
        }

        return b;
    }

    /**************************************************************************
     * @brief
     * OBB vs OBB overlap test using SAT (15 axes).
     * If overlapping, returns minimum-penetration normal from A -> B and depth.
     *
     * @param A, B
     * World-space OBBs.
     * @param outNormal
     * Minimum-penetration axis from A -> B (unit).
     * @param outPenetration
     * Minimum penetration depth along the axis (>= 0).
     *
     * @return
     * true if overlapping; outputs valid normal/penetration on success.
     **************************************************************************/
    bool CollisionSystem::computeOBBOBBCollision(OBB const &A, OBB const &B,
                                                 Vector3D &outNormal, float &outPenetration) {
        constexpr float EPS{ 1e-5f };

        Vector3D const &A0{ A.axis[0] }; Vector3D const &A1{ A.axis[1] }; Vector3D const &A2{ A.axis[2] };
        Vector3D const &B0{ B.axis[0] }; Vector3D const &B1{ B.axis[1] }; Vector3D const &B2{ B.axis[2] };

        float const a0{ A.half.x }, a1{ A.half.y }, a2{ A.half.z };
        float const b0{ B.half.x }, b1{ B.half.y }, b2{ B.half.z };

        float R[3][3]{
            { dot(A0,B0), dot(A0,B1), dot(A0,B2) },
            { dot(A1,B0), dot(A1,B1), dot(A1,B2) },
            { dot(A2,B0), dot(A2,B1), dot(A2,B2) }
        };

        float AbsR[3][3]{};
        for(int i{ 0 }; i < 3; ++i) {
            for(int j{ 0 }; j < 3; ++j) {
                AbsR[i][j] = std::fabs(R[i][j]) + EPS;
            }
        }

        Vector3D const tW{ sub(B.center, A.center) };
        float const tA[3]{ dot(tW, A0), dot(tW, A1), dot(tW, A2) };
        float const tB[3]{ dot(tW, B0), dot(tW, B1), dot(tW, B2) };

        float minOverlap{ std::numeric_limits<float>::max() };
        Vector3D bestAxis{ 0.0f, 0.0f, 0.0f };

        auto updateAxis = [&](Vector3D const &axis, float projDist, float ra, float rb) {
            float const overlap{ (ra + rb) - projDist };
            if(overlap < minOverlap) {
                minOverlap = overlap;
                bestAxis = axis;
            }
            };

        {
            float const ra0{ a0 };
            float const rb0{ b0 * AbsR[0][0] + b1 * AbsR[0][1] + b2 * AbsR[0][2] };
            float const dist0{ std::fabs(tA[0]) };
            if(dist0 > ra0 + rb0) {
                return false;
            }
            updateAxis((tA[0] >= 0.0f ? A0 : Vector3D{ -A0.x,-A0.y,-A0.z }), dist0, ra0, rb0);

            float const ra1_{ a1 };
            float const rb1_{ b0 * AbsR[1][0] + b1 * AbsR[1][1] + b2 * AbsR[1][2] };
            float const dist1_{ std::fabs(tA[1]) };
            if(dist1_ > ra1_ + rb1_) {
                return false;
            }
            updateAxis((tA[1] >= 0.0f ? A1 : Vector3D{ -A1.x,-A1.y,-A1.z }), dist1_, ra1_, rb1_);

            float const ra2_{ a2 };
            float const rb2_{ b0 * AbsR[2][0] + b1 * AbsR[2][1] + b2 * AbsR[2][2] };
            float const dist2_{ std::fabs(tA[2]) };
            if(dist2_ > ra2_ + rb2_) {
                return false;
            }
            updateAxis((tA[2] >= 0.0f ? A2 : Vector3D{ -A2.x,-A2.y,-A2.z }), dist2_, ra2_, rb2_);
        }

        {
            float const ra0{ a0 * AbsR[0][0] + a1 * AbsR[1][0] + a2 * AbsR[2][0] };
            float const rb0{ b0 };
            float const dist0{ std::fabs(tB[0]) };
            if(dist0 > ra0 + rb0) {
                return false;
            }
            updateAxis((tB[0] >= 0.0f ? B0 : Vector3D{ -B0.x,-B0.y,-B0.z }), dist0, ra0, rb0);

            float const ra1_{ a0 * AbsR[0][1] + a1 * AbsR[1][1] + a2 * AbsR[2][1] };
            float const rb1_{ b1 };
            float const dist1_{ std::fabs(tB[1]) };
            if(dist1_ > ra1_ + rb1_) {
                return false;
            }
            updateAxis((tB[1] >= 0.0f ? B1 : Vector3D{ -B1.x,-B1.y,-B1.z }), dist1_, ra1_, rb1_);

            float const ra2_{ a0 * AbsR[0][2] + a1 * AbsR[1][2] + a2 * AbsR[2][2] };
            float const rb2_{ b2 };
            float const dist2_{ std::fabs(tB[2]) };
            if(dist2_ > ra2_ + rb2_) {
                return false;
            }
            updateAxis((tB[2] >= 0.0f ? B2 : Vector3D{ -B2.x,-B2.y,-B2.z }), dist2_, ra2_, rb2_);
        }

        auto aAt = [&](int idx)->float { return (idx == 0 ? a0 : (idx == 1 ? a1 : a2)); };
        auto bAt = [&](int idx)->float { return (idx == 0 ? b0 : (idx == 1 ? b1 : b2)); };

        for(int i{ 0 }; i < 3; ++i) {
            for(int j{ 0 }; j < 3; ++j) {
                int const i1{ (i + 1) % 3 }, i2{ (i + 2) % 3 };
                int const j1{ (j + 1) % 3 }, j2{ (j + 2) % 3 };

                float const dist{ std::fabs(tA[i2] * R[i1][j] - tA[i1] * R[i2][j]) };
                float const ra{ aAt(i1) * AbsR[i2][j] + aAt(i2) * AbsR[i1][j] };
                float const rb{ bAt(j1) * AbsR[i][j2] + bAt(j2) * AbsR[i][j1] };

                if(dist > ra + rb) {
                    return false;
                }

                Vector3D axis{ cross(A.axis[i], B.axis[j]) };
                float const len2{ axis.x * axis.x + axis.y * axis.y + axis.z * axis.z };

                if(len2 > 1e-8f) {
                    float const s{ (dot(axis, tW) >= 0.0f) ? 1.0f : -1.0f };
                    axis = Vector3D{ axis.x * s, axis.y * s, axis.z * s };
                    updateAxis(axis, dist, ra, rb);
                }
            }
        }

        if(minOverlap == std::numeric_limits<float>::max()) {
            return false;
        }

        float const nLen{ std::sqrt(bestAxis.x * bestAxis.x + bestAxis.y * bestAxis.y + bestAxis.z * bestAxis.z) };
        if(nLen > 1e-6f) {
            outNormal = Vector3D{ bestAxis.x / nLen, bestAxis.y / nLen, bestAxis.z / nLen };
        }
        else {
            Vector3D n{ sub(B.center, A.center) };
            float const l{ std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z) };
            outNormal = (l > 1e-6f) ? Vector3D{ n.x / l, n.y / l, n.z / l } : Vector3D{ 1.0f, 0.0f, 0.0f };
        }

        outPenetration = minOverlap;
        if(outPenetration < 0.0f) {
            outPenetration = 0.0f;
        }
        return true;
    }

} // namespace gam300
