/******************************************************************************/
/*!
\file       PhysicsSystem.cpp
\author     Low Yue Jun (yuejun.low)
\date       Oct 03 2025
\brief      Implementation of PhysicsSystem for RigidBodyComponent.
/******************************************************************************/

#include "../System/PhysicsSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ECSManager.h"

// If you later apply angular rotation via quaternions, include glm when needed.
// #include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

namespace gam300
{
	PhysicsSystem::PhysicsSystem()
		: ComponentSystem<RigidBody>("PhysicsSystem")
		, PhysicsEcsRef(EM)
	{
		set_priority(101);
	}

	bool PhysicsSystem::init(SystemManager& /*sysMgr*/)
	{
		LM.writeLog("PhysicsSystem::init() - Physics System Initialized");
		return true;
	}

	void PhysicsSystem::update(float dt)
	{
		m_dt = dt;
		if (m_dt <= 0.0f) return;

		for (EntityID entity_id : m_entities)
		{
			process_entity(entity_id);
		}
	}


	void PhysicsSystem::shutdown()
	{
		LM.writeLog("PhysicsSystem::shutdown() - Physics System shut down");
	}

	void PhysicsSystem::process_entity(EntityID entity_id)
	{
		// Require both Transform3D and RigidBody
		if (!PhysicsEcsRef.hasComponent<Transform3D>(entity_id) ||
			!PhysicsEcsRef.hasComponent<RigidBody>(entity_id))
		{
			return;
		}

		Transform3D* transform = PhysicsEcsRef.getComponent<Transform3D>(entity_id);
		RigidBody* rigidBody = PhysicsEcsRef.getComponent<RigidBody>(entity_id);
		if (!transform || !rigidBody) return;

		rigidBody->Integrate(*transform, m_dt);
	}

} // namespace gam300
