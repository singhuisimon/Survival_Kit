/*****************************************************************************/
/*!
\file       Collider.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Collider component supporting a minimal AABB shape.
            World AABB is derived from Transform3D (position + local offset).

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once
#ifndef __COLLIDER_H__
#define __COLLIDER_H__

#include "Component.h"
#include "../Utility/Vector3D.h"

namespace gam300 {
    class Transform3D;

    /**************************************************************************
     * @brief
     * Minimal collider component exposing a local AABB and helpers
     * to compute a world-space AABB from a Transform.
     **************************************************************************/
    class Collider : public Component {
    public:
        /**************************************************************************
         * @brief
         * Local-space AABB description.
         **************************************************************************/
        struct AABBData {
            Vector3D halfExtents{ 0.0f, 0.0f, 0.0f };  //!< positive half sizes (x,y,z)
            Vector3D offset{ 0.0f, 0.0f, 0.0f };       //!< local center offset from Transform position
        };

        /**************************************************************************
         * @brief
         * Default-constructs a collider with zero half-extents and zero offset.
         **************************************************************************/
        Collider();

        /**************************************************************************
         * @brief
         * Constructs a collider from a local-space AABB description.
         *
         * @param aabb
         * Local-space AABB (half-extents and offset).
         **************************************************************************/
        explicit Collider(AABBData const &aabb);

        /**************************************************************************
         * @brief
         * Associates this component with an entity.
         *
         * @param entity_id
         * Target entity identifier.
         **************************************************************************/
        void init(EntityID /*entity_id*/) override;

        /**************************************************************************
         * @brief
         * Per-frame maintenance for the component (no-op by default).
         *
         * @param dt
         * Delta time in seconds.
         **************************************************************************/
        void update(float /*dt*/) override;

        // ---------------- AABB access ----------------

        /**************************************************************************
         * @brief
         * Returns the local-space AABB (const).
         *
         * @return
         * Const reference to AABBData.
         **************************************************************************/
        AABBData const &getAABB() const {
            return m_aabb;
        }

        /**************************************************************************
         * @brief
         * Returns the local-space AABB (mutable).
         *
         * @return
         * Reference to AABBData.
         **************************************************************************/
        AABBData &getAABB() {
            return m_aabb;
        }

        /**************************************************************************
         * @brief
         * Returns local-space half-extents (x,y,z).
         *
         * @return
         * Const reference to half-extents.
         **************************************************************************/
        Vector3D const &getAABBHalfExtents() const {
            return m_aabb.halfExtents;
        }

        /**************************************************************************
         * @brief
         * Returns local-space center offset from Transform position.
         *
         * @return
         * Const reference to offset.
         **************************************************************************/
        Vector3D const &getAABBOffset() const {
            return m_aabb.offset;
        }

        /**************************************************************************
         * @brief
         * Sets local-space half-extents (x,y,z).
         *
         * @param h
         * New half-extents (must be non-negative).
         **************************************************************************/
        void setAABBHalfExtents(Vector3D const &h) {
            m_aabb.halfExtents = h;
        }

        /**************************************************************************
         * @brief
         * Sets local-space center offset from Transform position.
         *
         * @param o
         * New local offset.
         **************************************************************************/
        void setAABBOffset(Vector3D const &o) {
            m_aabb.offset = o;
        }

        /**************************************************************************
         * @brief
         * Computes world-space AABB from a Transform and this collider’s local AABB.
         *
         * @param tr
         * Source transform providing position (and, if applicable in future, scale/rot).
         * @param outCenter
         * Output world-space AABB center.
         * @param outHalfExtents
         * Output world-space AABB half-extents.
         **************************************************************************/
        void getWorldAABB(Transform3D const &tr,
                          Vector3D &outCenter,
                          Vector3D &outHalfExtents) const;

    private:
        AABBData m_aabb{};
    };

} // namespace gam300

#endif // __COLLIDER_H__
