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
		const glm::vec3 GetLocalPosition()  const { return local_position; }

		const glm::quat GetRotation()  const { return rotation; }
		const glm::quat GetLocalRotation()  const { return local_rotation; }

		const glm::vec3 GetScale()     const { return scale; }
		const glm::vec3 GetLocalScale()     const { return local_scale; }

		const glm::mat4 GetTransform() const { return world_transform; }
		const glm::mat4 GetLocalTransform() const { return local_transform; }

		const EntityID  GetParentID() const { return parent; }

		const bool      IsDirty() const { return dirty; }

		void  SetPosition(glm::vec3  const& pos) { position = pos; }
		void  SetLocalPosition(glm::vec3  const& pos) { local_position = pos; }

		void  SetRotation(glm::quat  const& rot) { rotation = rot; }
		void  SetLocalRotation(glm::quat  const& rot) { local_rotation = rot; }

		void  SetScale(glm::vec3 const& scl)	 { scale = scl; }
		void  SetLocalScale(glm::vec3 const& scl) { local_scale = scl; }

		void  SetTransform(glm::mat4 const& trf) { world_transform = trf; }
		void  SetLocalTransform(glm::mat4 const& trf) { local_transform = trf; }

		void  SetParentID(EntityID const id) { parent = id; }

		void  MarkDirty(bool d) { dirty = d; }

	private:
		glm::vec3 local_position;
		glm::vec3 position;

		glm::vec3 local_scale;
		glm::vec3 scale;

		glm::quat rotation;
		glm::quat local_rotation;

		glm::mat4 world_transform;
		glm::mat4 local_transform;

		EntityID  parent = INVALID_ENTITY_ID;
		bool      dirty  = true;
	};
}

#endif // !__TRANSFORM_COMPONENT_H__
