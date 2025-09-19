#include "../System/TransformSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

namespace gam300 {

	TransformSystem::TransformSystem() : ComponentSystem<Transform3D>("TransformSystem") {
		set_priority(101);
	}

	bool TransformSystem::init(SystemManager&) {
		LM.writeLog("TransformSystem::init() - Transform System Initialized");
		return true;
	}

	void TransformSystem::update(float dt) {

		(void)dt;

		for (EntityID entity_id : m_entities) {
			process_entity(entity_id);
		}
	}

	void TransformSystem::shutdown() {
		LM.writeLog("TransformSystem::shutdown() - Transform System shut down");
	}

	void TransformSystem::process_entity(EntityID entity_id) {


	}

}