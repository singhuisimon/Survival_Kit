#include "../System/PhysicsSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ECSManager.h"
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

namespace gam300 {

	PhysicsSystem::PhysicsSystem() : ComponentSystem<RigidBody>("PhysicsSystem"), PhysicsEcsRef(EM) {
		//set_priority(101);
	}

	bool PhysicsSystem::init(SystemManager&) {

		//Find a way to register the system

		LM.writeLog("PhysicsSystem::init() - Physics System Initialized");
		return true;
	}

	void PhysicsSystem::update(float dt) {

		(void)dt;

		for (EntityID entity_id : m_entities) {
			process_entity(entity_id, dt);
		}
	}

	void PhysicsSystem::shutdown() {
		LM.writeLog("TransformSystem::shutdown() - Transform System shut down");
	}

	void PhysicsSystem::process_entity(EntityID entity_id, float dt) {
		if (PhysicsEcsRef.hasComponent<Transform3D>(entity_id) && PhysicsEcsRef.hasComponent<RigidBody>(entity_id)) {
			Transform3D* transform = PhysicsEcsRef.getComponent<Transform3D>(entity_id);
			RigidBody* rigidBody = PhysicsEcsRef.getComponent<RigidBody>(entity_id);

			rigidBody->clearAccumulators();
			rigidBody->integrateForces(dt);
			rigidBody->integrateVelocity(*transform, dt);
		}
	}

}