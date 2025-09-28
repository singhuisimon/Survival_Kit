#include "../System/PhysicsSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ECSManager.h"
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

namespace gam300 {

	PhysicsSystem::PhysicsSystem() : ComponentSystem<RigidBody>("PhysicsSystem"), PhysicsEcsRef(EM) {
		set_priority(101);
	}

	bool PhysicsSystem::init(SystemManager&) {

		JPH::RegisterDefaultAllocator();
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
		jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);

		// Create simple implementations
		auto* bpInterface = new SimpleBroadPhaseLayerInterface();
		auto* objVsBpFilter = new SimpleObjectVsBroadPhaseLayerFilter();
		auto* objPairFilter = new SimpleObjectLayerPairFilter();

		joltPhysics = new JPH::PhysicsSystem();
		joltPhysics->Init(1024, 0, 1024, 1024, *bpInterface, *objVsBpFilter, *objPairFilter);

		LM.writeLog("PhysicsSystem::init() - Physics System Initialized");
		return true;
	}

	void PhysicsSystem::update(float dt) {

		/*(void)dt;

		for (EntityID entity_id : m_entities) {
			process_entity(entity_id, dt);
		}*/

		
		joltPhysics->Update(dt, 1, tempAllocator, jobSystem);

		for (EntityID entity_id : m_entities) {
			if (PhysicsEcsRef.hasComponent<RigidBody>(entity_id)) {
				Transform3D* transform = PhysicsEcsRef.getComponent<Transform3D>(entity_id);
				RigidBody* rb = PhysicsEcsRef.getComponent<RigidBody>(entity_id);

				if (rb != nullptr && rb->getRigidBodyType() != BodyType::STATIC) {
					Vector3D pos = rb->getPosition();
					transform->setPosition(pos);
				}
			}
		}
		
	}

	void PhysicsSystem::shutdown() {
		LM.writeLog("TransformSystem::shutdown() - Transform System shut down");
	}

	void PhysicsSystem::process_entity(EntityID entity_id, float dt) {
		if (PhysicsEcsRef.hasComponent<Transform3D>(entity_id) && PhysicsEcsRef.hasComponent<RigidBody>(entity_id)) {
			Transform3D* transform = PhysicsEcsRef.getComponent<Transform3D>(entity_id);
			RigidBody* rigidBody = PhysicsEcsRef.getComponent<RigidBody>(entity_id);

			/*rigidBody->clearAccumulators();
			rigidBody->integrateForces(dt);
			rigidBody->integrateVelocity(*transform, dt);*/
		}
	}

}