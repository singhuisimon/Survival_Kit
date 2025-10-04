/******************************************************************************/
/*!
\file       Collider.h
\author     Low Yue Jun (yuejun.low)
\date       Oct 03 2025
\brief      Collider component that supports an AABB shape (minimal).
			World AABB is built off Transform3D (position + local offset).
/******************************************************************************/
#pragma once
#ifndef __COLLIDER_H__
#define __COLLIDER_H__

#include "Component.h"
#include "../Utility/Vector3D.h"

namespace gam300
{
	class Transform3D;

	class Collider : public Component
	{
	public:
		struct AABBData
		{
			Vector3D halfExtents;  // positive extents along x/y/z in local space
			Vector3D offset;       // local center offset relative to Transform3D::position
		};

		// Constructors
		Collider();
		explicit Collider(const AABBData &aabb);

		// Component interface
		void init(EntityID /*entity_id*/) override;
		void update(float /*dt*/) override;

		// AABB access
		const AABBData &getAABB() const { return m_aabb; }
		AABBData &getAABB() { return m_aabb; }

		// Convenience getters/setters
		const Vector3D &getAABBHalfExtents() const { return m_aabb.halfExtents; }
		const Vector3D &getAABBOffset()      const { return m_aabb.offset; }
		void setAABBHalfExtents(const Vector3D &h) { m_aabb.halfExtents = h; }
		void setAABBOffset(const Vector3D &o) { m_aabb.offset = o; }

		void getWorldAABB(const Transform3D &tr,
			Vector3D &outCenter,
			Vector3D &outHalfExtents) const;

	private:
		AABBData m_aabb;
	};
}

#endif // __COLLIDER_H__
