#include "../System/TransformSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"

#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

namespace gam300 {

	TransformSystem::TransformSystem() : ComponentSystem<TransformComponent>("TransformSystem") {
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

		// Get the transform component for this entity
		TransformComponent* transform_component = CM.get_component<TransformComponent>(entity_id);

		// Do the thing
		if (transform_component) {
			
			// Calculate the model to world transform
			glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), transform_component->GetScale());
			glm::mat4 rotation	   = glm::toMat4(transform_component->GetRotation());
		}
	}

}