/******************************************************************************/
/*!
\file       MovementControllerSystem.cpp
\author     Simon Chan
\date       Oct 04 2025
\brief      Implementation of MovementControllerSystem
/******************************************************************************/

#include "../System/MovementControllerSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/ECSManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ForceManager.h"
#include "../Manager/TorqueManager.h"
#include <cmath>

namespace gam300
{
    MovementControllerSystem::MovementControllerSystem()
        : ComponentSystem<MovementController, Transform3D>("MovementControllerSystem")
    {
        set_priority(95);
    }

    bool MovementControllerSystem::init(SystemManager& /*system_manager*/)
    {
        LM.writeLog("MovementControllerSystem::init() - Movement Controller System Initialized");
        return true;
    }

    void MovementControllerSystem::update(float dt)
    {
        m_dt = dt;

        for (EntityID entity_id : m_entities)
        {
            process_entity(entity_id);
        }
    }

    void MovementControllerSystem::shutdown()
    {
        LM.writeLog("MovementControllerSystem::shutdown() - System shut down");
    }

    void MovementControllerSystem::process_entity(EntityID entity_id)
    {
        auto controller = CM.get_component<MovementController>(entity_id);
        auto transform = CM.get_component<Transform3D>(entity_id);

        if (!controller || !transform) return;

        if (controller->getMovementMode() == MovementMode::DISABLED) {
            return;
        }

        auto rigidBody = CM.get_component<RigidBody>(entity_id);

        if (rigidBody && !rigidBody->isStatic())
        {
            applyDynamicMovement(entity_id, controller, transform, rigidBody);
        }
        else
        {
            applyKinematicMovement(entity_id, controller, transform);
        }

        controller->clearInputs();
    }

    void MovementControllerSystem::applyKinematicMovement(
        EntityID entity_id,
        MovementController* controller,
        Transform3D* transform)
    {
        (void)entity_id;

        const Vector3D& dir = controller->getDesiredDirection();
        float speed = controller->getKinematicSpeed();

        if (dir.x != 0.0f || dir.y != 0.0f || dir.z != 0.0f)
        {
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
            if (len > 0.0f)
            {
                Vector3D normalized(dir.x / len, dir.y / len, dir.z / len);
                Vector3D delta = normalized * speed * m_dt;
                transform->setPosition(transform->getPosition() + delta);
            }
        }

        const Vector3D& rot = controller->getDesiredRotation();
        if (rot.x != 0.0f || rot.y != 0.0f || rot.z != 0.0f)
        {
            Vector3D currentRot = transform->getRotation();
            transform->setRotation(currentRot + rot * m_dt);
        }
    }

    void MovementControllerSystem::applyDynamicMovement(
        EntityID entity_id,
        MovementController* controller,
        Transform3D* transform,
        RigidBody* rigidBody)
    {
        (void)entity_id;
        (void)transform;

        if ((rigidBody->getForceMask() & MOVEMENT_FORCE_MASK) == 0u)
            rigidBody->setForceMask(rigidBody->getForceMask() | MOVEMENT_FORCE_MASK);

        if ((rigidBody->getTorqueMask() & MOVEMENT_TORQUE_MASK) == 0u)
            rigidBody->setTorqueMask(rigidBody->getTorqueMask() | MOVEMENT_TORQUE_MASK);

        const Vector3D& dir = controller->getDesiredDirection();
        const float len2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;

        if (len2 > 1e-12f)
        {
            const float invLen = 1.0f / std::sqrt(len2);
            Vector3D normalized(dir.x * invLen, dir.y * invLen, dir.z * invLen);

            rigidBody->getForceManager().RemoveForcesByMask(MOVEMENT_FORCE_MASK);
            rigidBody->getForceManager().AddForce<LinearDirectionalForce>(
                normalized,
                controller->getMoveForce(),
                MOVEMENT_FORCE_MASK,
                Force::Perm
            );
        }
        else
        {
            rigidBody->getForceManager().RemoveForcesByMask(MOVEMENT_FORCE_MASK);
        }

        const Vector3D& rotDir = controller->getDesiredRotation();
        const float rotLen2 = rotDir.x * rotDir.x + rotDir.y * rotDir.y + rotDir.z * rotDir.z;

        if (rotLen2 > 1e-12f)
        {
            const float invRotLen = 1.0f / std::sqrt(rotLen2);
            Vector3D normalizedRot(
                rotDir.x * invRotLen,
                rotDir.y * invRotLen,
                rotDir.z * invRotLen
            );

            rigidBody->getTorqueManager().RemoveTorquesByMask(MOVEMENT_TORQUE_MASK);
            rigidBody->getTorqueManager().AddTorque<AngularDirectionalTorque>(
                normalizedRot,
                controller->getRotationTorque(),
                MOVEMENT_TORQUE_MASK,
                Torque::Perm
            );
        }
        else
        {
            rigidBody->getTorqueManager().RemoveTorquesByMask(MOVEMENT_TORQUE_MASK);
        }
    }

} // namespace gam300