#include "../System/TransformSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include <stack>

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

		std::vector<EntityID> roots;
		roots.reserve(m_entities.size());

		// Gather all the roots of the tree
		for (EntityID entity_id : m_entities) {
			Transform3D* trf = CM.get_component<Transform3D>(entity_id);
			if (!trf) 
				continue;
			
			if (trf->getParent() == INVALID_ENTITY_ID){
				roots.push_back(entity_id);
			}
		}

		// Iterate through all the roots
		for (EntityID root : roots) {
			Transform3D* trf = CM.get_component<Transform3D>(root);

			// If the root is dirty, recompute it's matrix
			if (trf->isDirty()) {
				glm::mat4 world_transform = trf->computeLocalTransformationMatrix();
				trf->setTransformationMatrix(world_transform);
			}

			propagate(root);
		}
	}

	void TransformSystem::shutdown() {
		LM.writeLog("TransformSystem::shutdown() - Transform System shut down");
	}

	void TransformSystem::process_entity(EntityID entity_id) {
		return;
	}

	void TransformSystem::propagate(EntityID root) {

		std::stack<EntityID> st;
		st.push(root);

		while (!st.empty()) {

			EntityID e = st.top(); st.pop();
			Transform3D* parent = CM.get_component<Transform3D>(e);

			glm::mat4 parent_matrix = parent->getTransformationMatrix();

			// Iterate through the children
			for (EntityID c : parent->getChildren()) {

				Transform3D* child = CM.get_component<Transform3D>(c);

				if (child->isDirty() || parent->isDirty()) {
					
					glm::mat4 child_local_matrix = child->computeLocalTransformationMatrix();
					glm::mat4 child_world_matrix = parent_matrix * child_local_matrix;
					child->setTransformationMatrix(child_world_matrix);
					child->markDirty(true);
				}

				st.push(c);
			}

			parent->markDirty(false);
		}

	}

}