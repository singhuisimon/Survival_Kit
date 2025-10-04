#pragma once

#ifndef __RENDER_COMPONENT_H__
#define __RENDER_COMPONENT_H__

#include "../Component/Component.h"
#include "../Graphics/SharedGraphics.h"

namespace gam300 {

	class RenderComponent : public Component {
	public:

		void init(EntityID entity_id) override;
		void update(float dt) override;

		inline u32 get_mesh_handle() const { return m_mesh_handle; }

	private:
		u32 m_mesh_handle = 0;
	};

}


#endif