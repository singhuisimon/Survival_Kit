/*****************************************************************************/
/*!
\file       Collider.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Minimal implementation for Collider component.

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "Collider.h"
#include "Transform3D.h"
#include <cmath>

namespace gam300 {
    /**************************************************************************
     * @brief
     * Default-constructs a collider with half-extents (0.5,0.5,0.5) and zero offset.
     **************************************************************************/
    Collider::Collider() :
        m_aabb{ Vector3D{ 0.5f, 0.5f, 0.5f }, Vector3D{ 0.0f, 0.0f, 0.0f } } {}

    /**************************************************************************
     * @brief
     * Constructs a collider from a local-space AABB.
     *
     * @param aabb
     * Local-space AABB (half-extents and offset).
     **************************************************************************/
    Collider::Collider(AABBData const &aabb) :
        m_aabb{ aabb } {}

    /**************************************************************************
     * @brief
     * Associates this component with an entity.
     *
     * @param entity_id
     * Target entity identifier.
     **************************************************************************/
    void Collider::init(EntityID /*entity_id*/) {}

    /**************************************************************************
     * @brief
     * Per-frame maintenance for the component (no-op).
     *
     * @param dt
     * Delta time in seconds.
     **************************************************************************/
    void Collider::update(float /*dt*/) {}

    /**************************************************************************
     * @brief
     * Computes world-space AABB from a Transform and this collider’s local AABB.
     *
     * @param tr
     * Source transform providing position and scale.
     * @param outCenter
     * Output world-space AABB center.
     * @param outHalfExtents
     * Output world-space AABB half-extents.
     **************************************************************************/
    void Collider::getWorldAABB(Transform3D const &tr,
                                Vector3D &outCenter,
                                Vector3D &outHalfExtents) const {
        Vector3D const pos{ tr.getPosition() };
        outCenter = Vector3D{ pos.x + m_aabb.offset.x,
                              pos.y + m_aabb.offset.y,
                              pos.z + m_aabb.offset.z };

        Vector3D const s{ tr.getScale() };
        outHalfExtents = Vector3D{ m_aabb.halfExtents.x * std::fabs(s.x),
                                   m_aabb.halfExtents.y * std::fabs(s.y),
                                   m_aabb.halfExtents.z * std::fabs(s.z) };
    }
}
