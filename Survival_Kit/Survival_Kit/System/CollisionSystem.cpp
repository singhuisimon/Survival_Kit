/*****************************************************************************/
/*!
\file       CollisionSystem.cpp
\author     Low Yue Jun (yuejun.low)
\date       Oct 03 2025
\brief      SAP broadphase + SAT(OBB) narrowphase; AABB/SAT contact & resolution.
*/
/*****************************************************************************/

#include "../System/CollisionSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

#include <cmath>
#include <cfloat>
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>
#include <glm-0.9.9.8/glm/gtc/matrix_transform.hpp>

namespace gam300 {
    // ---------------- System plumbing ----------------

    CollisionSystem::CollisionSystem()
        : ComponentSystem<Transform3D, Collider>("CollisionSystem") {
        set_priority(102);
    }

    bool CollisionSystem::init(SystemManager & /*system_manager*/) {
        LM.writeLog("CollisionSystem::init() - Collision System Initialized");
        return true;
    }

    void CollisionSystem::shutdown() {
        LM.writeLog("CollisionSystem::shutdown() - Collision System shut down");
    }

    // ---------------- Update ----------------

    void CollisionSystem::update(float dt) {
        m_dt = dt;
        const auto &ids = m_entities;
        const size_t n = ids.size();
        if(n < 2) return;

        // -------- Broadphase: Sweep & Prune on X --------
        std::vector<Proxy> proxies;
        proxies.reserve(n);
        for(EntityID e : ids) {
            AABB box = buildWorldAABB(e);
            const float minHalf = 1e-4f;
            box.half.x = clampMin(std::fabs(box.half.x), minHalf);
            box.half.y = clampMin(std::fabs(box.half.y), minHalf);
            box.half.z = clampMin(std::fabs(box.half.z), minHalf);

            Proxy p;
            p.id = e;
            p.box = box;
            p.minX = box.center.x - box.half.x;
            p.maxX = box.center.x + box.half.x;
            proxies.push_back(p);
        }

        std::sort(proxies.begin(), proxies.end(),
                  [](const Proxy &a, const Proxy &b) { return a.minX < b.minX; });

        std::vector<std::pair<EntityID, EntityID>> candidates;
        candidates.reserve(n * 2);

        for(size_t i = 0; i < proxies.size(); ++i) {
            const Proxy &A = proxies[i];
            for(size_t j = i + 1; j < proxies.size(); ++j) {
                const Proxy &B = proxies[j];
                if(B.minX > A.maxX) break; // no more overlaps on X
                if(aabbOverlap3D(A.box, B.box))
                    candidates.emplace_back(A.id, B.id);
            }
        }

        // -------- Narrowphase: SAT OBB vs OBB --------
        for(auto [ea, eb] : candidates) {
            const OBB obbA = buildWorldOBB(ea);
            const OBB obbB = buildWorldOBB(eb);

            Vector3D normal(0.f, 0.f, 0.f);
            float    penetration = 0.f;
            if(!computeOBBOBBCollision(obbA, obbB, normal, penetration))
                continue;

            // Resolve
            Contact c{ ea, eb, normal, penetration };
            resolve(c);
        }
    }

    // ---------------- Broadphase AABB ----------------

    CollisionSystem::AABB CollisionSystem::buildWorldAABB(EntityID e) {
        // Prefer Collider’s world AABB
        if(auto tr = CM.get_component<Transform3D>(e))
            if(auto col = CM.get_component<Collider>(e)) {
                Vector3D c, he;
                col->getWorldAABB(*tr, c, he);
                return { c, he };
            }

        // Fallback if no Collider: use position + scale*0.5
        Vector3D center = { 0,0,0 }, half = { 0.5f,0.5f,0.5f };
        if(auto tr = CM.get_component<Transform3D>(e)) {
            center = tr->getPosition();
            Vector3D s = tr->getScale();
            half = { std::max(std::fabs(0.5f * s.x), 1e-4f),
                     std::max(std::fabs(0.5f * s.y), 1e-4f),
                     std::max(std::fabs(0.5f * s.z), 1e-4f) };
        }
        return { center, half };
    }

    // ---------------- AABB narrowphase (optional/fallback) ----------------

    bool CollisionSystem::computeAABBCollision(const Transform3D &trA, const Collider &colA,
                                               const Transform3D &trB, const Collider &colB,
                                               Vector3D &outNormal, float &outPenetration) {
        Vector3D cA, eA, cB, eB;
        colA.getWorldAABB(trA, cA, eA);
        colB.getWorldAABB(trB, cB, eB);

        const Vector3D d(cB.x - cA.x, cB.y - cA.y, cB.z - cA.z);

        const float ox = (eA.x + eB.x) - std::fabs(d.x);
        if(ox <= 0.0f) return false;

        const float oy = (eA.y + eB.y) - std::fabs(d.y);
        if(oy <= 0.0f) return false;

        const float oz = (eA.z + eB.z) - std::fabs(d.z);
        if(oz <= 0.0f) return false;

        // Choose axis of minimum penetration
        outPenetration = ox;
        outNormal = Vector3D(signf(d.x), 0.0f, 0.0f);

        if(oy < outPenetration) {
            outPenetration = oy;
            outNormal = Vector3D(0.0f, signf(d.y), 0.0f);
        }
        if(oz < outPenetration) {
            outPenetration = oz;
            outNormal = Vector3D(0.0f, 0.0f, signf(d.z));
        }
        return true;
    }

    // ---------------- Resolution ----------------

    void CollisionSystem::resolve(Contact &c) {
        auto tA = CM.get_component<Transform3D>(c.a);
        auto tB = CM.get_component<Transform3D>(c.b);
        auto rbA = CM.get_component<RigidBody>(c.a);
        auto rbB = CM.get_component<RigidBody>(c.b);
        if(!tA || !tB) return;

        positionalCorrection(c, tA, rbA, tB, rbB);
        applyImpulse(c, rbA, rbB);
    }

    void CollisionSystem::positionalCorrection(const Contact &c, Transform3D *tA, RigidBody *rbA,
                                               Transform3D *tB, RigidBody *rbB) {
        const float invA = (rbA ? rbA->getInvMass() : 0.0f);
        const float invB = (rbB ? rbB->getInvMass() : 0.0f);
        const float invSum = invA + invB;
        if(invSum <= 0.0f) return; // both static

        // Baumgarte stabilization
        float correctionMag = (c.penetration - m_posSlop);
        if(correctionMag < 0.0f) correctionMag = 0.0f;
        correctionMag = (correctionMag / invSum) * m_posCorrectPercent;

        const Vector3D corrA(-c.normal.x * correctionMag * invA,
                             -c.normal.y * correctionMag * invA,
                             -c.normal.z * correctionMag * invA);

        const Vector3D corrB(c.normal.x * correctionMag * invB,
                             c.normal.y * correctionMag * invB,
                             c.normal.z * correctionMag * invB);

        Vector3D pA = tA->getPosition();
        pA.x += corrA.x; pA.y += corrA.y; pA.z += corrA.z;
        tA->setPosition(pA);

        Vector3D pB = tB->getPosition();
        pB.x += corrB.x; pB.y += corrB.y; pB.z += corrB.z;
        tB->setPosition(pB);
    }

    void CollisionSystem::applyImpulse(const Contact &c, RigidBody *rbA, RigidBody *rbB) {
        if(!rbA && !rbB) return;

        const float invA = (rbA ? rbA->getInvMass() : 0.0f);
        const float invB = (rbB ? rbB->getInvMass() : 0.0f);
        const float invSum = invA + invB;
        if(invSum <= 0.0f) return;

        const Vector3D vA = rbA ? rbA->getVelocity() : Vector3D(0.0f, 0.0f, 0.0f);
        const Vector3D vB = rbB ? rbB->getVelocity() : Vector3D(0.0f, 0.0f, 0.0f);

        const Vector3D rv(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
        const float relVelN = rv.x * c.normal.x + rv.y * c.normal.y + rv.z * c.normal.z;
        if(relVelN > 0.0f) return;

        const float e = m_restitution;
        const float j = -(1.0f + e) * relVelN / invSum;

        const Vector3D impulse(c.normal.x * j, c.normal.y * j, c.normal.z * j);

        if(rbA) {
            Vector3D nv(rbA->getVelocity());
            nv.x -= impulse.x * invA;
            nv.y -= impulse.y * invA;
            nv.z -= impulse.z * invA;
            rbA->setVelocity(nv);
        }
        if(rbB) {
            Vector3D nv(rbB->getVelocity());
            nv.x += impulse.x * invB;
            nv.y += impulse.y * invB;
            nv.z += impulse.z * invB;
            rbB->setVelocity(nv);
        }
    }

    // ---------------- Basis / OBB ----------------

    void CollisionSystem::extractBasis(const Transform3D &tr, Vector3D &x, Vector3D &y, Vector3D &z) {
        // Rotation-only quaternion (Z * Y * X) to match Transform3D
        const Vector3D e = tr.getRotation();
        const glm::quat qx = glm::angleAxis(glm::radians(e.x), glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::quat qy = glm::angleAxis(glm::radians(e.y), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::quat qz = glm::angleAxis(glm::radians(e.z), glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::quat q = qz * qy * qx;

        const glm::mat3 R = glm::mat3_cast(q);   // rotation only

        // Columns of R are world-space basis vectors (orthonormal)
        const glm::vec3 Rx = glm::vec3(R[0][0], R[1][0], R[2][0]);
        const glm::vec3 Ry = glm::vec3(R[0][1], R[1][1], R[2][1]);
        const glm::vec3 Rz = glm::vec3(R[0][2], R[1][2], R[2][2]);

        x = Vector3D(Rx.x, Rx.y, Rx.z).normalize();  // right
        y = Vector3D(Ry.x, Ry.y, Ry.z).normalize();  // up
        z = Vector3D(Rz.x, Rz.y, Rz.z).normalize();  // forward
    }

    CollisionSystem::OBB CollisionSystem::buildWorldOBB(EntityID e) {
        OBB b{};
        b.center = { 0,0,0 };
        b.axis[0] = { 1,0,0 };
        b.axis[1] = { 0,1,0 };
        b.axis[2] = { 0,0,1 };
        b.half = { 0.5f,0.5f,0.5f };

        if(auto tr = CM.get_component<Transform3D>(e)) {
            b.center = tr->getPosition();
            extractBasis(*tr, b.axis[0], b.axis[1], b.axis[2]);

            // Use transform scale as half-extents for the OBB proxy
            const Vector3D s = tr->getScale();
            const float eps = 1e-4f;
            b.half = {
                std::max(std::fabs(0.5f * s.x), eps),
                std::max(std::fabs(0.5f * s.y), eps),
                std::max(std::fabs(0.5f * s.z), eps)
            };
        }
        return b;
    }

    // ---------------- SAT: OBB vs OBB ----------------

    static inline float absf(float v) {
        return v >= 0.0f ? v : -v;
    }

    bool CollisionSystem::computeOBBOBBCollision(const OBB &A, const OBB &B,
                                                 Vector3D &outNormal, float &outPenetration) {
        // Based on "OBB-OBB overlap test using SAT" (Gottschalk/Van Den Bergen)
        constexpr float EPS = 1e-5f;

        // Convenience aliases
        const Vector3D &A0 = A.axis[0]; const Vector3D &A1 = A.axis[1]; const Vector3D &A2 = A.axis[2];
        const Vector3D &B0 = B.axis[0]; const Vector3D &B1 = B.axis[1]; const Vector3D &B2 = B.axis[2];

        const float a0 = A.half.x, a1 = A.half.y, a2 = A.half.z;
        const float b0 = B.half.x, b1 = B.half.y, b2 = B.half.z;

        // Rotation matrix R = dot(Ai, Bj)
        float R[3][3] = {
            { dot(A0,B0), dot(A0,B1), dot(A0,B2) },
            { dot(A1,B0), dot(A1,B1), dot(A1,B2) },
            { dot(A2,B0), dot(A2,B1), dot(A2,B2) }
        };
        float AbsR[3][3];
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                AbsR[i][j] = std::fabs(R[i][j]) + EPS; // add epsilon to handle near-parallel axes

        // Translation T from A to B expressed in A's basis
        const Vector3D tW = sub(B.center, A.center);
        float tA[3] = { dot(tW, A0), dot(tW, A1), dot(tW, A2) };
        // Also in B's basis (for the Bj tests)
        float tB[3] = { dot(tW, B0), dot(tW, B1), dot(tW, B2) };

        // Track minimal penetration axis (for contact)
        float minOverlap = FLT_MAX;
        Vector3D bestAxis = { 0,0,0 };

        auto updateAxis = [&](const Vector3D &axis, float projDist, float ra, float rb) {
            const float overlap = (ra + rb) - projDist;
            if(overlap < minOverlap) {
                minOverlap = overlap;
                bestAxis = axis; // not necessarily unit for cross-axes; we will normalize later
            }
            };

        // ---- 1) Axes A0, A1, A2 ----
        {
            float ra, rb, dist;

            // A0
            ra = a0;
            rb = b0 * AbsR[0][0] + b1 * AbsR[0][1] + b2 * AbsR[0][2];
            dist = std::fabs(tA[0]);
            if(dist > ra + rb) return false;
            updateAxis((tA[0] >= 0.f ? A0 : Vector3D{ -A0.x,-A0.y,-A0.z }), dist, ra, rb);

            // A1
            ra = a1;
            rb = b0 * AbsR[1][0] + b1 * AbsR[1][1] + b2 * AbsR[1][2];
            dist = std::fabs(tA[1]);
            if(dist > ra + rb) return false;
            updateAxis((tA[1] >= 0.f ? A1 : Vector3D{ -A1.x,-A1.y,-A1.z }), dist, ra, rb);

            // A2
            ra = a2;
            rb = b0 * AbsR[2][0] + b1 * AbsR[2][1] + b2 * AbsR[2][2];
            dist = std::fabs(tA[2]);
            if(dist > ra + rb) return false;
            updateAxis((tA[2] >= 0.f ? A2 : Vector3D{ -A2.x,-A2.y,-A2.z }), dist, ra, rb);
        }

        // ---- 2) Axes B0, B1, B2 ----
        {
            float ra, rb, dist;

            // B0
            ra = a0 * AbsR[0][0] + a1 * AbsR[1][0] + a2 * AbsR[2][0];
            rb = b0;
            dist = std::fabs(tB[0]);
            if(dist > ra + rb) return false;
            updateAxis((tB[0] >= 0.f ? B0 : Vector3D{ -B0.x,-B0.y,-B0.z }), dist, ra, rb);

            // B1
            ra = a0 * AbsR[0][1] + a1 * AbsR[1][1] + a2 * AbsR[2][1];
            rb = b1;
            dist = std::fabs(tB[1]);
            if(dist > ra + rb) return false;
            updateAxis((tB[1] >= 0.f ? B1 : Vector3D{ -B1.x,-B1.y,-B1.z }), dist, ra, rb);

            // B2
            ra = a0 * AbsR[0][2] + a1 * AbsR[1][2] + a2 * AbsR[2][2];
            rb = b2;
            dist = std::fabs(tB[2]);
            if(dist > ra + rb) return false;
            updateAxis((tB[2] >= 0.f ? B2 : Vector3D{ -B2.x,-B2.y,-B2.z }), dist, ra, rb);
        }

        // ---- 3) Axes Ai x Bj (9 tests) ----
        // Use formulas from GOTTSCHALK (OBBTree) / Van Den Bergen:
        //   dist = | tA[(i+2)%3]*R[(i+1)%3][j] - tA[(i+1)%3]*R[(i+2)%3][j] |
        //   ra   = a[(i+1)%3]*AbsR[(i+2)%3][j] + a[(i+2)%3]*AbsR[(i+1)%3][j]
        //   rb   = b[(j+1)%3]*AbsR[i][(j+2)%3] + b[(j+2)%3]*AbsR[i][(j+1)%3]
        auto aAt = [&](int idx)->float { return (idx == 0 ? a0 : (idx == 1 ? a1 : a2)); };
        auto bAt = [&](int idx)->float { return (idx == 0 ? b0 : (idx == 1 ? b1 : b2)); };

        for(int i = 0; i < 3; ++i) {
            for(int j = 0; j < 3; ++j) {
                const int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
                const int j1 = (j + 1) % 3, j2 = (j + 2) % 3;

                const float dist = std::fabs(tA[i2] * R[i1][j] - tA[i1] * R[i2][j]);
                const float ra = aAt(i1) * AbsR[i2][j] + aAt(i2) * AbsR[i1][j];
                const float rb = bAt(j1) * AbsR[i][j2] + bAt(j2) * AbsR[i][j1];

                if(dist > ra + rb) return false;

                // Candidate normal is cross(Ai, Bj); orient to point from A->B
                Vector3D axis = cross(A.axis[i], B.axis[j]);
                const float len2 = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
                if(len2 > 1e-8f) {
                    // Orient normal so dot(n, tW) >= 0
                    const float s = (dot(axis, tW) >= 0.f) ? 1.f : -1.f;
                    axis = { axis.x * s, axis.y * s, axis.z * s };
                    updateAxis(axis, dist, ra, rb);
                }
                // If nearly parallel (axis ~ 0), the other tests will dominate; skip.
            }
        }

        // If we got here: overlap on all 15 axes
        if(minOverlap == FLT_MAX) return false; // should not happen

        // Normalize best axis, output penetration
        const float nLen = std::sqrt(bestAxis.x * bestAxis.x + bestAxis.y * bestAxis.y + bestAxis.z * bestAxis.z);
        if(nLen > 1e-6f) {
            outNormal = { bestAxis.x / nLen, bestAxis.y / nLen, bestAxis.z / nLen };
        }
        else {
            // Degenerate; fall back to center direction
            Vector3D n = sub(B.center, A.center);
            const float l = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
            outNormal = (l > 1e-6f) ? Vector3D{ n.x / l, n.y / l, n.z / l } : Vector3D{ 1,0,0 };
        }
        outPenetration = minOverlap;
        if(outPenetration < 0.f) outPenetration = 0.f; // numeric guard
        return true;
    }
}
