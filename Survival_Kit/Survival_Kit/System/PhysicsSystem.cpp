/*****************************************************************************/
/*!
\file       PhysicsSystem.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Implementation of PhysicsSystem for RigidBody component.

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "../System/PhysicsSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ECSManager.h"

namespace gam300 {
    /**************************************************************************
     * @brief
     * Constructs the physics system and sets scheduling priority.
     **************************************************************************/
    PhysicsSystem::PhysicsSystem() :
        ComponentSystem<RigidBody>{ "PhysicsSystem" },
        PhysicsEcsRef{ EM }
    {
        set_priority(101);
    }

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
    bool PhysicsSystem::init(SystemManager &/*system_manager*/) {
        LM.writeLog("PhysicsSystem::init() - Physics System Initialized");
        return true;
    }

    /**************************************************************************
     * @brief
     * Steps physics integration for all registered entities.
     *
     * @param dt
     * Delta time in seconds.
     **************************************************************************/
    void PhysicsSystem::update(float dt) {
        m_dt = dt;
        if(m_dt <= 0.0f) {
            return;
        }

        for(EntityID entity_id : m_entities) {
            process_entity(entity_id);
        }
    }

    /**************************************************************************
     * @brief
     * Shuts down and logs teardown.
     **************************************************************************/
    void PhysicsSystem::shutdown() {
        LM.writeLog("PhysicsSystem::shutdown() - Physics System shut down");
    }

    /**************************************************************************
     * @brief
     * Integrates a single entity’s rigid body using its transform.
     *
     * @param entity_id
     * Target entity identifier.
     **************************************************************************/
    void PhysicsSystem::process_entity(EntityID entity_id) {
        if(!PhysicsEcsRef.hasComponent<Transform3D>(entity_id) ||
           !PhysicsEcsRef.hasComponent<RigidBody>(entity_id)) {
            return;
        }

        Transform3D *transform = PhysicsEcsRef.getComponent<Transform3D>(entity_id);
        RigidBody *rigidBody = PhysicsEcsRef.getComponent<RigidBody>(entity_id);
        if(!transform || !rigidBody) {
            return;
        }

        rigidBody->Integrate(*transform, m_dt);
    }

} // namespace gam300
