#pragma once

#ifndef __TRANSFORM_COMPONENT_H__
#define __TRANSFORM_COMPONENT_H__

#include <glm-0.9.9.8/glm/glm.hpp>
#include <glm-0.9.9.8/glm/gtc/quaternion.hpp>
#include "Component.h"

namespace gam300{

	class TransformComponent : public Component {

	public:

		const glm::vec3 GetPosition()  const { return position; }
		const glm::quat GetRotation()  const { return rotation; }
		const glm::vec3 GetScale()     const { return scale; }
		const glm::mat4 GetTransform() const { return world_transform; }

		void  SetPosition(glm::vec3  const& pos) { position = pos; }
		void  SetRotation(glm::quat  const& rot) { rotation = rot; }
		void  SetScale(glm::vec3 const& scl)	 { scale = scl; }
		void  SetTransform(glm::mat4 const& trf) { world_transform = trf; }

	private:
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		glm::mat4 world_transform;
		EntityID  parent = INVALID_ENTITY_ID;
	};
}

#endif // !__TRANSFORM_COMPONENT_H__
