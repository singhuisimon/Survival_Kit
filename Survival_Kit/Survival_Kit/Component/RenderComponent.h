#pragma once

#ifndef __RENDER_COMPONENT_H__
#define __RENDER_COMPONENT_H__

#include "../Component/Component.h"

#include "../Graphics/SharedGraphics.h"

namespace gam300 {

	class RenderComponent : public Component {
	public:
	
		RenderComponent() : m_mesh_handle(0) { }
		void init(EntityID entity_id) override;
		inline u64 getHandle() { return m_mesh_handle; }

	private:
		u64 m_mesh_handle;
	};

}


#endif