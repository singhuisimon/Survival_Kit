/******************************************************************************/
/*!
\file       MovementSystem.cpp
\author     (you)
\date       Oct 03 2025
\brief      Rewired to use Transform3D + RigidBodyComponent (ForceManager-based).
/******************************************************************************/

#include "../System/MovementSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/InputManager.h"

// If your key defines live elsewhere, keep using IM.isKeyPressed(GLFW_KEY_*)

namespace gam300
{
    MovementSystem::MovementSystem()
        : ComponentSystem<Transform3D, RigidBody>("MovementSystem")
    {
        set_priority(100);
    }

    bool MovementSystem::init(SystemManager & /*system_manager*/)
    {
        LM.writeLog("MovementSystem::init() - Movement System Initialized");
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
        if (!transform || !rigidBody) return;

        // --- KINEMATIC-style control: treat mass==0 as kinematic (direct transform edits) ---
        if (rigidBody->isStatic())
        {
            // WASD moves in X/Y plane (match your previous behavior)
            if (IM.isKeyPressed(GLFW_KEY_A))
            {
                transform->setPosition(transform->getPosition() + Vector3D(-2.0f, 0.0f, 0.0f) * m_dt);
            }
            if (IM.isKeyPressed(GLFW_KEY_D))
            {
                transform->setPosition(transform->getPosition() + Vector3D(2.0f, 0.0f, 0.0f) * m_dt);
            }
            if (IM.isKeyPressed(GLFW_KEY_W))
            {
                transform->setPosition(transform->getPosition() + Vector3D(0.0f, 2.0f, 0.0f) * m_dt);
            }
            if (IM.isKeyPressed(GLFW_KEY_S))
            {
                transform->setPosition(transform->getPosition() + Vector3D(0.0f, -2.0f, 0.0f) * m_dt);
            }
            return;
        }

        // --- DYNAMIC: convert input to forces for this frame ---
        // Clear any previous frame's input forces (so they only last while keys are held)
        rigidBody->getForceManager().RemoveForcesByMask(INPUT_FORCE_MASK);

        // Sum up desired input directions (unit vectors)
        Vector3D dir(0.0f, 0.0f, 0.0f);
        if (IM.isKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_W)) dir.y += 1.0f;  // same as your earlier example (Y up)
        if (IM.isKeyPressed(GLFW_KEY_S)) dir.y -= 1.0f;

        // Normalize if needed
        const float len2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        if (len2 > 1e-12f)
        {
            const float invLen = 1.0f / std::sqrt(len2);
            dir.x *= invLen; dir.y *= invLen; dir.z *= invLen;

            // Add a directional force tagged with our INPUT_FORCE_MASK.
            // We remove/re-add every frame, so duration doesn't matter (Perm is fine).
            rigidBody->getForceManager().AddForce<LinearDirectionalForce>(
                dir, m_moveForce, INPUT_FORCE_MASK, Force::Perm
            );

            // Ensure physics considers these forces this frame:
            // PhysicsSystem will call rb->Integrate(*transform, m_dt).
            // (Integration uses ForceManager inside RigidBodyComponent.)
        }

        // OPTIONAL: quick test impulse or velocity tweak (commented to avoid double-accel)
        // Vector3D impulse = Vector3D(2.0f, 0.0f, 0.0f) * m_dt * rigidBody->getMass();
        // rigidBody->setVelocity(rigidBody->getVelocity() + impulse * rigidBody->getInvMass());
    }
}
