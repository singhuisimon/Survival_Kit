/******************************************************************************/
/*!
\file       RigidBody.h
\author     (you)
\date       Oct 03 2025
\brief      Rigid body component with linear + angular forces (mask-based).
			Inherits Component. Provides full getters/setters. Linear motion
			writes position in Integrate(); angular state is updated here,
			but rotation write-back is left to the caller/system.
/******************************************************************************/
#pragma once
#ifndef RigidBody_H
#define RigidBody_H

#include "../Component/Component.h"
#include "../Utility/Vector3D.h"
#include "../Manager/ForceManager.h"
#include "../Manager/TorqueManager.h"

namespace gam300
{
	struct Transform3D; // forward decl

	class RigidBody : public Component
	{
	public:
		RigidBody(
			float          mass = 0.0f,
			const Vector3D &velocity = Vector3D(0.0f, 0.0f, 0.0f),
			const Vector3D &acceleration = Vector3D(0.0f, 0.0f, 0.0f),
			const Vector3D &inertiaDiag = Vector3D(0.0f, 0.0f, 0.0f),
			const Vector3D &angularVelocity = Vector3D(0.0f, 0.0f, 0.0f),
			unsigned        forceMask = 0xFFFFFFFFu,
			unsigned        torqueMask = 0xFFFFFFFFu,
			int             layer = 0
		);

		// ---- Component interface ----
		void init(EntityID entity_id) override;
		void update(float dt) override;

		// ---- Integration entry point (call from your physics pass) ----
		void Integrate(Transform3D &tr, float dt);

		// ---- Getters / Setters (linear) ----
		float          getMass() const { return m_mass; }
		float          getInvMass() const { return m_invMass; }
		const Vector3D &getVelocity() const { return m_velocity; }
		const Vector3D &getAcceleration() const { return m_acceleration; }
		unsigned       getForceMask() const { return m_forceMask; }
		int            getLayer() const { return m_layer; }
		ForceManager &getForceManager() { return m_forceMgr; }
		const ForceManager &getForceManager() const { return m_forceMgr; }

		void setMass(float m);
		void setVelocity(const Vector3D &v) { m_velocity = v; }
		void setAcceleration(const Vector3D &a) { m_acceleration = a; }
		void setForceMask(unsigned mask) { m_forceMask = mask; }
		void setLayer(int layer) { m_layer = layer; }

		// ---- Getters / Setters (angular) ----
		const Vector3D &getInertiaDiagonal() const { return m_inertiaDiag; }
		const Vector3D &getInvInertiaDiagonal() const { return m_invInertiaDiag; }
		const Vector3D &getAngularVelocity() const { return m_angularVelocity; }
		const Vector3D &getAngularAcceleration() const { return m_angularAcceleration; }
		unsigned        getTorqueMask() const { return m_torqueMask; }
		TorqueManager &getTorqueManager() { return m_torqueMgr; }
		const TorqueManager &getTorqueManager() const { return m_torqueMgr; }

		void setInertiaDiagonal(const Vector3D &I);
		void setAngularVelocity(const Vector3D &w) { m_angularVelocity = w; }
		void setAngularAcceleration(const Vector3D &a) { m_angularAcceleration = a; }
		void setTorqueMask(unsigned mask) { m_torqueMask = mask; }

		// convenience
		bool isStatic() const { return m_invMass <= 0.0f; }
		Vector3D GetAngularDelta(float dt) const
		{
			return Vector3D(m_angularVelocity.x * dt,
				m_angularVelocity.y * dt,
				m_angularVelocity.z * dt);
		}

	private:
		static inline float inv_or_zero(float x) { return (x > 0.0f) ? (1.0f / x) : 0.0f; }

		// linear
		float    m_mass;
		float    m_invMass;
		Vector3D m_velocity;
		Vector3D m_acceleration;
		unsigned m_forceMask;
		int      m_layer;
		ForceManager m_forceMgr;

		// angular
		Vector3D m_inertiaDiag;
		Vector3D m_invInertiaDiag;
		Vector3D m_angularVelocity;
		Vector3D m_angularAcceleration;
		unsigned m_torqueMask;
		TorqueManager m_torqueMgr;
	};

} // namespace gam300

#endif // RigidBody_H
