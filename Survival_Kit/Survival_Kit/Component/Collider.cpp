/******************************************************************************/
/*!
\file       Collider.cpp
\author     Low Yue Jun (yuejun.low)
\date       Oct 03 2025
\brief      Minimal implementation for Collider component.
/******************************************************************************/

#include "Collider.h"
#include "Transform3D.h"

namespace gam300
{
	Collider::Collider()
		: m_aabb{ Vector3D(0.5f, 0.5f, 0.5f), Vector3D(0.0f, 0.0f, 0.0f) }
	{}

	Collider::Collider(const AABBData &aabb)
		: m_aabb(aabb)
	{}

	void Collider::init(EntityID /*entity_id*/)
	{
		// No-op
	}

	void Collider::update(float /*dt*/)
	{
		// No-op
	}

	void Collider::getWorldAABB(const Transform3D &tr,
		Vector3D &outCenter,
		Vector3D &outHalfExtents) const
	{
		const Vector3D pos = tr.getPosition();
		outCenter = Vector3D(pos.x + m_aabb.offset.x,
			pos.y + m_aabb.offset.y,
			pos.z + m_aabb.offset.z);

		// If you later expose scale on Transform3D, multiply here:
		Vector3D s = tr.getScale();
		outHalfExtents = Vector3D(m_aabb.halfExtents.x * std::abs(s.x), 
			m_aabb.halfExtents.y * std::abs(s.y), 
			m_aabb.halfExtents.z * std::abs(s.z));
	}
}
