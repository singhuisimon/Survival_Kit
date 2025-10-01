/******************************************************************************/
/*!
\file       CollisionSystem.cpp
\author     (you)
\date       Oct 03 2025
\brief      Minimal AABB vs AABB detection + resolution using Transform-built colliders.
/******************************************************************************/

#include "../System/CollisionSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include <cmath> // fabsf

namespace gam300
{
	// ---------------- System plumbing ----------------

	CollisionSystem::CollisionSystem()
		: ComponentSystem<Transform3D, Collider>("CollisionSystem")
	{
		set_priority(102);
	}

	bool CollisionSystem::init(SystemManager & /*system_manager*/)
	{
		LM.writeLog("CollisionSystem::init() - Collision System Initialized");
		return true;
	}

	void CollisionSystem::shutdown()
	{
		LM.writeLog("CollisionSystem::shutdown() - Collision System shut down");
	}

	void CollisionSystem::update(float dt)
	{
		m_dt = dt;
		const auto &ids = m_entities;
		const size_t n = ids.size();
		if (n < 2) return;

		for (size_t i = 0; i + 1 < n; ++i)
		{
			const EntityID ea = ids[i];
			auto tA = CM.get_component<Transform3D>(ea);
			auto cA = CM.get_component<Collider>(ea);
			if (!tA || !cA) continue;

			for (size_t j = i + 1; j < n; ++j)
			{
				const EntityID eb = ids[j];
				auto tB = CM.get_component<Transform3D>(eb);
				auto cB = CM.get_component<Collider>(eb);
				if (!tB || !cB) continue;

				Vector3D normal(0.0f, 0.0f, 0.0f);
				float penetration = 0.0f;

				if (computeAABBCollision(*tA, *cA, *tB, *cB, normal, penetration))
				{
					Contact c{ ea, eb, normal, penetration };
					resolve(c);
				}
			}
		}
	}

	// ---------------- AABB detection (built from Transform) ----------------

	bool CollisionSystem::computeAABBCollision(const Transform3D &trA, const Collider &colA,
		const Transform3D &trB, const Collider &colB,
		Vector3D &outNormal, float &outPenetration)
	{
		Vector3D cA, eA, cB, eB;
		colA.getWorldAABB(trA, cA, eA);
		colB.getWorldAABB(trB, cB, eB);

		const Vector3D d(cB.x - cA.x, cB.y - cA.y, cB.z - cA.z);

		const float ox = (eA.x + eB.x) - std::fabs(d.x);
		if (ox <= 0.0f) return false;

		const float oy = (eA.y + eB.y) - std::fabs(d.y);
		if (oy <= 0.0f) return false;

		const float oz = (eA.z + eB.z) - std::fabs(d.z);
		if (oz <= 0.0f) return false;

		// Choose axis of minimum penetration
		outPenetration = ox;
		outNormal = Vector3D(signf(d.x), 0.0f, 0.0f);

		if (oy < outPenetration)
		{
			outPenetration = oy;
			outNormal = Vector3D(0.0f, signf(d.y), 0.0f);
		}
		if (oz < outPenetration)
		{
			outPenetration = oz;
			outNormal = Vector3D(0.0f, 0.0f, signf(d.z));
		}

		return true;
	}

	// ---------------- Resolution ----------------

	void CollisionSystem::resolve(Contact &c)
	{
		auto tA = CM.get_component<Transform3D>(c.a);
		auto tB = CM.get_component<Transform3D>(c.b);
		auto rbA = CM.get_component<RigidBody>(c.a);
		auto rbB = CM.get_component<RigidBody>(c.b);
		if (!tA || !tB) return;

		positionalCorrection(c, tA, rbA, tB, rbB);
		applyImpulse(c, rbA, rbB);
	}

	void CollisionSystem::positionalCorrection(const Contact &c, Transform3D *tA, RigidBody *rbA,
		Transform3D *tB, RigidBody *rbB)
	{
		const float invA = (rbA ? rbA->getInvMass() : 0.0f);
		const float invB = (rbB ? rbB->getInvMass() : 0.0f);
		const float invSum = invA + invB;

		if (invSum <= 0.0f) return; // both static

		// Baumgarte stabilization
		float correctionMag = (c.penetration - m_posSlop);
		if (correctionMag < 0.0f) correctionMag = 0.0f;
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

	void CollisionSystem::applyImpulse(const Contact &c, RigidBody *rbA, RigidBody *rbB)
	{
		if (!rbA && !rbB) return;

		const float invA = (rbA ? rbA->getInvMass() : 0.0f);
		const float invB = (rbB ? rbB->getInvMass() : 0.0f);
		const float invSum = invA + invB;
		if (invSum <= 0.0f) return;

		const Vector3D vA = rbA ? rbA->getVelocity() : Vector3D(0.0f, 0.0f, 0.0f);
		const Vector3D vB = rbB ? rbB->getVelocity() : Vector3D(0.0f, 0.0f, 0.0f);

		const Vector3D rv(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
		const float relVelN = rv.x * c.normal.x + rv.y * c.normal.y + rv.z * c.normal.z;
		if (relVelN > 0.0f) return;

		const float e = m_restitution;
		const float j = -(1.0f + e) * relVelN / invSum;

		const Vector3D impulse(c.normal.x * j, c.normal.y * j, c.normal.z * j);

		if (rbA)
		{
			Vector3D nv(rbA->getVelocity());
			nv.x -= impulse.x * invA;
			nv.y -= impulse.y * invA;
			nv.z -= impulse.z * invA;
			rbA->setVelocity(nv);
		}
		if (rbB)
		{
			Vector3D nv(rbB->getVelocity());
			nv.x += impulse.x * invB;
			nv.y += impulse.y * invB;
			nv.z += impulse.z * invB;
			rbB->setVelocity(nv);
		}
	}
}
