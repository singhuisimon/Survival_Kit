/*****************************************************************************/
/*!
\file       RigidBody.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Implementation for RigidBody (linear + angular).

			(C) 2025 DigiPen Institute of Technology.
			Reproduction or disclosure of this file or its contents without the
			prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "RigidBody.h"
#include "../Component/Transform3D.h"

namespace gam300 {
	/**************************************************************************
	 * @brief
	 * Constructs a rigid body with optional initial linear and angular state.
	 *
	 * @param mass
	 * Mass in kilograms (<= 0 makes the body static).
	 * @param velocity
	 * Initial linear velocity (m/s).
	 * @param acceleration
	 * Initial linear acceleration (m/s^2).
	 * @param inertiaDiag
	 * Local-space diagonal inertia tensor (x,y,z).
	 * @param angularVelocity
	 * Initial angular velocity (rad/s).
	 * @param forceMask
	 * Bitmask gate for linear forces.
	 * @param torqueMask
	 * Bitmask gate for torques.
	 * @param layer
	 * Collision/logic layer.
	 **************************************************************************/
	RigidBody::RigidBody(
		float              mass,
		Vector3D const &velocity,
		Vector3D const &acceleration,
		Vector3D const &inertiaDiag,
		Vector3D const &angularVelocity,
		unsigned           forceMask,
		unsigned           torqueMask,
		int                layer
	)
		: m_mass{ mass }
		, m_invMass{ inv_or_zero(mass) }
		, m_velocity{ velocity }
		, m_acceleration{ acceleration }
		, m_forceMask{ forceMask }
		, m_layer{ layer }
		, m_inertiaDiag{ inertiaDiag }
		, m_invInertiaDiag{ inv_or_zero(inertiaDiag.x), inv_or_zero(inertiaDiag.y), inv_or_zero(inertiaDiag.z) }
		, m_angularVelocity{ angularVelocity }
		, m_angularAcceleration{ 0.0f, 0.0f, 0.0f }
		, m_torqueMask{ torqueMask } {}

	/**************************************************************************
	 * @brief
	 * Associates this component instance with an entity.
	 *
	 * @param
	 * entity_id Target entity identifier.
	 **************************************************************************/
	void RigidBody::init(EntityID) {}

	/**************************************************************************
	 * @brief
	 * Per-frame maintenance for the component (if needed).
	 *
	 * @param dt
	 * Delta time in seconds.
	 **************************************************************************/
	void RigidBody::update(float) {}

	/**************************************************************************
	 * @brief
	 * Sets mass (<= 0 makes the body static) and updates inverse mass.
	 *
	 * @param m
	 * Mass in kilograms.
	 **************************************************************************/
	void RigidBody::setMass(float m) {
		m_mass = m;
		m_invMass = inv_or_zero(m);
	}

	/**************************************************************************
	 * @brief
	 * Sets local-space diagonal inertia and updates its inverse.
	 *
	 * @param I
	 * Inertia diagonal (x,y,z).
	 **************************************************************************/
	void RigidBody::setInertiaDiagonal(Vector3D const &I) {
		m_inertiaDiag = I;
		m_invInertiaDiag = Vector3D{ inv_or_zero(I.x), inv_or_zero(I.y), inv_or_zero(I.z) };
	}

	/**************************************************************************
	 * @brief
	 * Performs one semi-implicit Euler step for linear state and updates
	 * angular state. Rotation write-back is applied to the Transform.
	 *
	 * @param tr
	 * Transform to write updated position/rotation into.
	 * @param dt
	 * Delta time in seconds.
	 **************************************************************************/
	void RigidBody::Integrate(Transform3D &tr, float dt) {
		if(dt <= 0.0f) {
			return;
		}

		m_forceMgr.SetCurrentVelocity(m_velocity);

		Vector3D totalF{ 0.0f, 0.0f, 0.0f };
		if(!isStatic() && m_forceMask != 0u) {
			Vector3D const f{ m_forceMgr.CalculateForceByMask(m_forceMask) };
			totalF.x += f.x; totalF.y += f.y; totalF.z += f.z;
		}

		if(!isStatic()) {
			m_acceleration.x = totalF.x * m_invMass;
			m_acceleration.y = totalF.y * m_invMass;
			m_acceleration.z = totalF.z * m_invMass;

			m_velocity.x += m_acceleration.x * dt;
			m_velocity.y += m_acceleration.y * dt;
			m_velocity.z += m_acceleration.z * dt;

			Vector3D pos{ tr.getPosition() };
			pos.x += m_velocity.x * dt;
			pos.y += m_velocity.y * dt;
			pos.z += m_velocity.z * dt;
			tr.setPosition(pos);
		}
		else {
			m_acceleration = Vector3D{ 0.0f, 0.0f, 0.0f };
		}

		m_forceMgr.CleanupForces();

		m_torqueMgr.SetCurrentAngularVelocity(m_angularVelocity);

		Vector3D T{ 0.0f, 0.0f, 0.0f };
		if(m_torqueMask != 0u) {
			T = m_torqueMgr.CalculateTorqueByMask(m_torqueMask);
		}

		if(m_invInertiaDiag.x == 0.0f && m_invInertiaDiag.y == 0.0f && m_invInertiaDiag.z == 0.0f) {
			m_inertiaDiag = Vector3D{ 1.0f, 1.0f, 1.0f };
			m_invInertiaDiag = Vector3D{ 1.0f, 1.0f, 1.0f };
		}

		m_angularAcceleration.x = T.x * m_invInertiaDiag.x;
		m_angularAcceleration.y = T.y * m_invInertiaDiag.y;
		m_angularAcceleration.z = T.z * m_invInertiaDiag.z;

		m_angularVelocity.x += m_angularAcceleration.x * dt;
		m_angularVelocity.y += m_angularAcceleration.y * dt;
		m_angularVelocity.z += m_angularAcceleration.z * dt;

		Vector3D r{ tr.getRotation() };
		r.x += m_angularVelocity.x * dt;
		r.y += m_angularVelocity.y * dt;
		r.z += m_angularVelocity.z * dt;
		tr.setRotation(r);

		m_torqueMgr.CleanupTorques();
	}
}
