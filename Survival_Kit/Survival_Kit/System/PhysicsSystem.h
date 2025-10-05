/*****************************************************************************/
/*!
\file       PhysicsSystem.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Physics system that updates RigidBody each frame.
            - Finds (Transform3D, RigidBody) pairs
            - Calls Integrate() on each with stored dt

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once
#ifndef __PHYSICS_SYSTEM_H__
#define __PHYSICS_SYSTEM_H__

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"

namespace gam300 {
    class SystemManager;
    class ECSManager;

    /**************************************************************************
     * @brief
     * System that steps rigid-body integration each frame.
     *
     * @details
     * Iterates entities with RigidBody, fetches paired Transform3D, and calls
     * RigidBody::Integrate() using the delta time stored on update().
     **************************************************************************/
    class PhysicsSystem : public ComponentSystem<RigidBody> {
    public:
        /**************************************************************************
         * @brief
         * Constructs the physics system and sets scheduling priority.
         **************************************************************************/
        PhysicsSystem();

        /**************************************************************************
         * @brief
         * Initializes internal state and logs startup.
         *
         * @param system_manager
         * Reference to SystemManager (unused here).
         *
         * @return
         * true if successful; otherwise false.
         **************************************************************************/
        bool init(SystemManager &system_manager) override;

        /**************************************************************************
         * @brief
         * Stores dt and processes all registered entities for integration.
         *
         * @param dt
         * Delta time in seconds.
         **************************************************************************/
        void update(float dt) override;

        /**************************************************************************
         * @brief
         * Releases resources and logs teardown.
         **************************************************************************/
        void shutdown() override;

        /**************************************************************************
         * @brief
         * Integrates a single entity’s rigid body using its transform.
         *
         * @param entity_id
         * Target entity identifier.
         **************************************************************************/
        void process_entity(EntityID entity_id) override;

    private:
        ECSManager &PhysicsEcsRef;   //!< Bound to EM in ctor
        float       m_dt{ 0.0f };    //!< Cached delta time
    };

} // namespace gam300

#endif // __PHYSICS_SYSTEM_H__
