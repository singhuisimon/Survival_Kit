/******************************************************************************/
/*!
\file       MovementSystem.cpp
\author     (you)
\date       Oct 03 2025
\brief      Applies direct transform edits for static bodies, and stacks
		   persistent forces for dynamic bodies via RigidBody's ForceManager.
/******************************************************************************/

#include "../System/MovementSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/InputManager.h"

// We create LinearDirectionalForce and use Force::Perm.
// With stacking enabled, each frame a key is held adds another permanent force.
#include "../Manager/ForceManager.h"

#include <cmath> // std::sqrt

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

        // ---------- STATIC: mass==0 -> direct transform edits ----------
        if (rigidBody->isStatic())
        {
            Vector3D delta(0.0f, 0.0f, 0.0f);
            if (IM.isKeyPressed(GLFW_KEY_A)) delta.x -= m_kinematicSpeed * m_dt;
            if (IM.isKeyPressed(GLFW_KEY_D)) delta.x += m_kinematicSpeed * m_dt;
            if (IM.isKeyPressed(GLFW_KEY_W)) delta.y += m_kinematicSpeed * m_dt; // Y-up as before
            if (IM.isKeyPressed(GLFW_KEY_S)) delta.y -= m_kinematicSpeed * m_dt;

            if (delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f)
                transform->setPosition(transform->getPosition() + delta);
            return;
        }

        // ---------- DYNAMIC: replace any old force with new one ----------
        // Ensure our mask bit is enabled
        if ((rigidBody->getForceMask() & INPUT_FORCE_MASK) == 0u)
            rigidBody->setForceMask(rigidBody->getForceMask() | INPUT_FORCE_MASK);

        // Build desired input direction
        Vector3D dir(0.0f, 0.0f, 0.0f);
        if (IM.isKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_W)) dir.y += 1.0f;
        if (IM.isKeyPressed(GLFW_KEY_S)) dir.y -= 1.0f;

        const float len2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        if (len2 > 1e-12f)
        {
            // Normalize direction
            const float invLen = 1.0f / std::sqrt(len2);
            dir.x *= invLen; dir.y *= invLen; dir.z *= invLen;

            // Clear any previous input forces
            rigidBody->getForceManager().RemoveForcesByMask(INPUT_FORCE_MASK);

            // Add the new one
            rigidBody->getForceManager().AddForce<LinearDirectionalForce>(
                dir, m_moveForce, INPUT_FORCE_MASK, Force::Perm
            );
        }
        else
        {
            rigidBody->getForceManager().RemoveForcesByMask(INPUT_FORCE_MASK);
        }
    }

}
