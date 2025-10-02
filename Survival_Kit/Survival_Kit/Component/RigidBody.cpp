/******************************************************************************/
/*!
\file       RigidBody.cpp
\author     (you)
\date       Oct 03 2025
\brief      Implementation for RigidBody (linear + angular).
/******************************************************************************/
#include "RigidBody.h"
#include "../Component/Transform3D.h"

namespace gam300
{
	RigidBody::RigidBody(
		float           mass,
		const Vector3D& velocity,
		const Vector3D& acceleration,
		const Vector3D& inertiaDiag,
		const Vector3D& angularVelocity,
		unsigned        forceMask,
		unsigned        torqueMask,
		int             layer
	)
		: m_mass(mass)
		, m_invMass(inv_or_zero(mass))
		, m_velocity(velocity)
		, m_acceleration(acceleration)
		, m_forceMask(forceMask)
		, m_layer(layer)
		, m_inertiaDiag(inertiaDiag)
		, m_invInertiaDiag(inv_or_zero(inertiaDiag.x), inv_or_zero(inertiaDiag.y), inv_or_zero(inertiaDiag.z))
		, m_angularVelocity(angularVelocity)
		, m_angularAcceleration(0.0f, 0.0f, 0.0f)
		, m_torqueMask(torqueMask)
	{
	}

	void RigidBody::init(EntityID)
	{
	}

	void RigidBody::update(float)
	{
	}

	void RigidBody::setMass(float m)
	{
		m_mass = m;
		m_invMass = inv_or_zero(m);
	}

	void RigidBody::setInertiaDiagonal(const Vector3D& I)
	{
		m_inertiaDiag = I;
		m_invInertiaDiag = Vector3D(inv_or_zero(I.x), inv_or_zero(I.y), inv_or_zero(I.z));
	}

	void RigidBody::Integrate(Transform3D& tr, float dt)
	{
		if (dt <= 0.0f) return;

		// -------- linear --------
		m_forceMgr.SetCurrentVelocity(m_velocity);

		Vector3D totalF(0.0f, 0.0f, 0.0f);
		if (!isStatic() && m_forceMask != 0u)
		{
			// If your ForceManager lacks CalculateForceByMask, swap for GetTotalForce()
			const Vector3D f = m_forceMgr.CalculateForceByMask(m_forceMask);
			totalF.x += f.x; totalF.y += f.y; totalF.z += f.z;
		}

		if (!isStatic())
		{
			m_acceleration.x = totalF.x * m_invMass;
			m_acceleration.y = totalF.y * m_invMass;
			m_acceleration.z = totalF.z * m_invMass;

			m_velocity.x += m_acceleration.x * dt;
			m_velocity.y += m_acceleration.y * dt;
			m_velocity.z += m_acceleration.z * dt;

			Vector3D pos = tr.getPosition();
			pos.x += m_velocity.x * dt;
			pos.y += m_velocity.y * dt;
			pos.z += m_velocity.z * dt;
			tr.setPosition(pos);
		}
		else
		{
			m_acceleration = Vector3D(0.0f, 0.0f, 0.0f);
		}

		// Remove only inactive Temp forces; Perm stay
		m_forceMgr.CleanupForces();

		// -------- angular --------
		m_torqueMgr.SetCurrentAngularVelocity(m_angularVelocity);

		Vector3D T(0.0f, 0.0f, 0.0f);
		if (m_torqueMask != 0u)
		{
			T = m_torqueMgr.CalculateTorqueByMask(m_torqueMask);
		}

		// If all inverse inertias are zero, choose a harmless default so torque has effect.
		if (m_invInertiaDiag.x == 0.0f && m_invInertiaDiag.y == 0.0f && m_invInertiaDiag.z == 0.0f)
		{
			m_inertiaDiag = Vector3D(1.0f, 1.0f, 1.0f);
			m_invInertiaDiag = Vector3D(1.0f, 1.0f, 1.0f);
		}

		// alpha = I^-1 * T  (diagonal inertia)
		m_angularAcceleration.x = T.x * m_invInertiaDiag.x;
		m_angularAcceleration.y = T.y * m_invInertiaDiag.y;
		m_angularAcceleration.z = T.z * m_invInertiaDiag.z;

		m_angularVelocity.x += m_angularAcceleration.x * dt;
		m_angularVelocity.y += m_angularAcceleration.y * dt;
		m_angularVelocity.z += m_angularAcceleration.z * dt;

		// Euler add (swap to quat path if your Transform exposes quats)
		Vector3D r = tr.getRotation();
		r.x += m_angularVelocity.x * dt;
		r.y += m_angularVelocity.y * dt;
		r.z += m_angularVelocity.z * dt;
		tr.setRotation(r);

		// Remove only inactive Temp torques; Perm stay
		m_torqueMgr.CleanupTorques();
	}
}
