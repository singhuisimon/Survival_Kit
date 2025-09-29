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
		const Vector3D &velocity,
		const Vector3D &acceleration,
		const Vector3D &inertiaDiag,
		const Vector3D &angularVelocity,
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
	{}

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

	void RigidBody::setInertiaDiagonal(const Vector3D &I)
	{
		m_inertiaDiag = I;
		m_invInertiaDiag = Vector3D(inv_or_zero(I.x), inv_or_zero(I.y), inv_or_zero(I.z));
	}

    void RigidBody::Integrate(Transform3D &tr, float dt)
    {
        if (dt <= 0.0f) return;

        // -------- linear --------
        // Provide current velocity so drag-like forces can use it.
        m_forceMgr.SetCurrentVelocity(m_velocity);

        Vector3D totalF(0.0f, 0.0f, 0.0f);

        // Sum all active forces whose mask intersects with this body's force mask.
        if (m_forceMask != 0u)
        {
            const Vector3D f = m_forceMgr.CalculateForceByMask(m_forceMask);
            totalF.x += f.x; totalF.y += f.y; totalF.z += f.z;
        }

        // a = F * invMass (skip if static/massless)
        if (!isStatic())
        {
            m_acceleration.x = totalF.x * m_invMass;
            m_acceleration.y = totalF.y * m_invMass;
            m_acceleration.z = totalF.z * m_invMass;

            // v += a*dt
            m_velocity.x += m_acceleration.x * dt;
            m_velocity.y += m_acceleration.y * dt;
            m_velocity.z += m_acceleration.z * dt;

            // x += v*dt (use accessors to avoid relying on public fields)
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

        // Cleanup removes only inactive Temp forces; Perm ones remain and keep stacking.
        m_forceMgr.CleanupForces();

        // -------- angular --------
        m_torqueMgr.SetCurrentAngularVelocity(m_angularVelocity);

        Vector3D totalTau(0.0f, 0.0f, 0.0f);
        if (m_torqueMask != 0u)
        {
            const Vector3D tau = m_torqueMgr.CalculateTorqueByMask(m_torqueMask);
            totalTau.x += tau.x; totalTau.y += tau.y; totalTau.z += tau.z;
        }

        // alpha = I^-1 .* tau (per-axis; zero inv inertia => locked axis)
        m_angularAcceleration.x = totalTau.x * m_invInertiaDiag.x;
        m_angularAcceleration.y = totalTau.y * m_invInertiaDiag.y;
        m_angularAcceleration.z = totalTau.z * m_invInertiaDiag.z;

        // omega += alpha * dt
        m_angularVelocity.x += m_angularAcceleration.x * dt;
        m_angularVelocity.y += m_angularAcceleration.y * dt;
        m_angularVelocity.z += m_angularAcceleration.z * dt;

        // NOTE: rotation write-back (Euler/quaternion) is left to caller/system.
        // m_torqueMgr.CleanupTorques() removes only inactive Temp torques.
        m_torqueMgr.CleanupTorques();
    }

}
