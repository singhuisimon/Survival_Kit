#include "../System/PhysicsSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/ECSManager.h"
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>


namespace gam300
{
	PhysicsSystem::PhysicsSystem() : ComponentSystem < RigidBody>("PhysicsSystem")
	{
		set_priority(50);
	}

	bool PhysicsSystem::init(SystemManager& system_manager) {

		////Find a way to register the system
		////system_manager.register_system("MovementSystem");

		//JPH::RegisterDefaultAllocator();
		//JPH::Factory::sInstance = new JPH::Factory();
		//JPH::RegisterTypes();

		////tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
		////jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);

		//////// Create simple implementations
		////auto* bpInterface = new SimpleBroadPhaseLayerInterface();
		////auto* objVsBpFilter = new SimpleObjectVsBroadPhaseLayerFilter();
		////auto* objPairFilter = new SimpleObjectLayerPairFilter();

		////joltPhysics = new JPH::PhysicsSystem();
		////joltPhysics->Init(1024, 0, 1024, 1024, *bpInterface, *objVsBpFilter, *objPairFilter);

		//LM.writeLog("PhysicsSystem::init() - Physics System Initialized");
		//return true;

		//tempAllocator = new JPH::TempAllocatorImpl(16 * 1024 * 1024);

		//// Create job system thread pool
		//jobSystem = new JPH::JobSystemThreadPool();
		//jobSystem->Init(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
		//	std::thread::hardware_concurrency() - 1);

		//// Create broad phase layer interface
		//broadPhaseLayerInterface = new SimpleBroadPhaseLayerInterface();

		//// Initialize physics system
		//joltPhysics = new JPH::PhysicsSystem();
		//joltPhysics->Init(1024, 0, 1024, 1024,
		//	*broadPhaseLayerInterface,
		//	SimpleObjectVsBroadPhaseLayerFilter(),
		//	SimpleObjectLayerPairFilter());

		LM.writeLog("PhysicsSystem::init() - Physics System Initialized");
		return true;

	}
	void PhysicsSystem::update(float dt) {

		(void)dt;
		m_dt = dt;
		for (EntityID entity_id : m_entities) {
			process_entity(entity_id);
		}
	}
	void PhysicsSystem::shutdown() {
		/*	if (joltPhysics) {
				delete joltPhysics;
				joltPhysics = nullptr;
			}

			if (jobSystem) {
				delete jobSystem;
				jobSystem = nullptr;
			}

			if (tempAllocator) {
				delete tempAllocator;
				tempAllocator = nullptr;
			}

			if (broadPhaseLayerInterface) {
				delete broadPhaseLayerInterface;
				broadPhaseLayerInterface = nullptr;
			}*/
		LM.writeLog("PhysicsSystem::shutdown() - PhysicsSystem shut down");
	}
	void PhysicsSystem::process_entity(EntityID entity_id) {


	}


} // end of namespace gam300