#include "../System/MovementSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

namespace gam300 {

	MovementSystem::MovementSystem() : ComponentSystem<Transform3D>("MovementSystem") {
		//set_priority(101);
	}

	bool MovementSystem::init(SystemManager& system_manager) {

		//Find a way to register the system
		//system_manager.register_system("MovementSystem");

		LM.writeLog("MovementSystem::init() - Movement System Initialized");
		return true;
	}

	void MovementSystem::update(float dt) {

		(void)dt;

		for (EntityID entity_id : m_entities) {
			process_entity(entity_id);
		}
	}

	void MovementSystem::shutdown() {
		LM.writeLog("TransformSystem::shutdown() - Transform System shut down");
	}

	void MovementSystem::process_entity(EntityID entity_id) {


	}

}