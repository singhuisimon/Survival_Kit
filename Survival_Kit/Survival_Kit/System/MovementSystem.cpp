/******************************************************************************/
/*!
\file       MovementSystem.cpp
\author     (you)
\date       Oct 04 2025
\brief      Refactored movement system that applies movement based on
            MovementController settings (scriptable or direct input).
/******************************************************************************/

#include "../System/MovementSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ForceManager.h"
#include "../Manager/TorqueManager.h"
#include <cmath>

namespace gam300
{
    MovementSystem::MovementSystem()
        : ComponentSystem<Transform3D, RigidBody, MovementController>("MovementSystem")
    {
        set_priority(100);
    }

    bool MovementSystem::init(SystemManager& /*system_manager*/)
    {
        LM.writeLog("MovementSystem::init() - Movement System Initialized (Scriptable)");
        return true;
    }

    void MovementSystem::update(float dt)
    {
        m_dt = dt;
        for (EntityID entity_id : m_entities)
        {
            process_entity(entity_id);
        }
    }

    void MovementSystem::shutdown()
    {
        LM.writeLog("MovementSystem::shutdown() - MovementSystem shut down");
    }

    void MovementSystem::process_entity(EntityID entity_id)
    {
        auto transform = CM.get_component<Transform3D>(entity_id);
        auto rigidBody = CM.get_component<RigidBody>(entity_id);
        auto controller = CM.get_component<MovementController>(entity_id);

        if (!transform || !rigidBody || !controller) return;

        // Check movement mode
        MovementMode mode = controller->getMovementMode();

        if (mode == MovementMode::DISABLED)
        {
            return; // No movement processing
        }

        // If using direct input mode, process input first
        if (mode == MovementMode::DIRECT_INPUT)
        {
            processDirectInput(entity_id, transform, rigidBody, controller);
        }

        // At this point, desired direction/rotation should be set
        // (either by script or by direct input processing above)

        // Apply movement based on body type
        if (rigidBody->isStatic())
        {
            applyKinematicMovement(transform, controller);
        }
        else
        {
            applyDynamicMovement(rigidBody, controller);
            applyDynamicRotation(rigidBody, controller);
        }

        // Clear inputs for next frame (scripts will set them again)
        controller->clearInputs();
    }

    void MovementSystem::applyKinematicMovement(Transform3D* transform, MovementController* controller)
    {
        Vector3D dir = controller->getDesiredDirection();

        if (dir.x != 0.0f || dir.y != 0.0f || dir.z != 0.0f)
        {
            Vector3D delta = dir * controller->getKinematicSpeed() * m_dt;
            transform->setPosition(transform->getPosition() + delta);
        }
    }

    void MovementSystem::applyDynamicMovement(RigidBody* rigidBody, MovementController* controller)
    {
        // Ensure linear input mask is enabled
        if ((rigidBody->getForceMask() & INPUT_FORCE_MASK) == 0u)
            rigidBody->setForceMask(rigidBody->getForceMask() | INPUT_FORCE_MASK);

        Vector3D dir = controller->getDesiredDirection();
        const float len2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;

        if (len2 > 1e-12f)
        {
            // Normalize direction
            const float invLen = 1.0f / std::sqrt(len2);
            dir.x *= invLen;
            dir.y *= invLen;
            dir.z *= invLen;

            // Clear previous input forces and add new one
            rigidBody->getForceManager().RemoveForcesByMask(INPUT_FORCE_MASK);
            rigidBody->getForceManager().AddForce<LinearDirectionalForce>(
                dir, controller->getMoveForce(), INPUT_FORCE_MASK, Force::Perm
            );
        }
        else
        {
            // No linear input -> remove forces
            rigidBody->getForceManager().RemoveForcesByMask(INPUT_FORCE_MASK);
        }
    }

    void MovementSystem::applyDynamicRotation(RigidBody* rigidBody, MovementController* controller)
    {
        // Ensure torque input mask is enabled
        if ((rigidBody->getTorqueMask() & INPUT_TORQUE_MASK) == 0u)
            rigidBody->setTorqueMask(rigidBody->getTorqueMask() | INPUT_TORQUE_MASK);

        Vector3D rotAxis = controller->getDesiredRotation();
        const float len2 = rotAxis.x * rotAxis.x + rotAxis.y * rotAxis.y + rotAxis.z * rotAxis.z;

        if (len2 > 1e-12f)
        {
            // Clear previous input torques and add new one
            rigidBody->getTorqueManager().RemoveTorquesByMask(INPUT_TORQUE_MASK);
            rigidBody->getTorqueManager().AddTorque<AngularDirectionalTorque>(
                rotAxis, controller->getRotationTorque(), INPUT_TORQUE_MASK, Torque::Perm
            );
        }
        else
        {
            // No rotation input -> remove torques
            rigidBody->getTorqueManager().RemoveTorquesByMask(INPUT_TORQUE_MASK);
        }
    }

    void MovementSystem::processDirectInput(EntityID entity_id, Transform3D* transform,
        RigidBody* rigidBody, MovementController* controller)
    {
        (void)entity_id;
        (void)transform;
        (void)rigidBody;

        // Legacy direct input handling (WASD movement)
        Vector3D dir(0.0f, 0.0f, 0.0f);

        if (IM.isKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_W)) dir.y += 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_S)) dir.y -= 1.0f;

        controller->setDesiredDirection(dir);

        // R key for rotation
        if (IM.isKeyPressed(GLFW_KEY_R))
        {
            controller->setDesiredRotation(Vector3D(1.0f, 1.0f, 1.0f));
        }
    }
}