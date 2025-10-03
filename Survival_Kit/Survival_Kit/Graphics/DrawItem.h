#pragma once

#ifndef __DRAW_ITEM_H__
#define __DRAW_ITEM_H__
#include "../Graphics/Common.h"
#include <glm-0.9.9.8/glm/glm.hpp>

namespace gam300{

	struct DrawItem
	{
		u16       m_mesh_handle;
		u16       m_texture_handle;
		u16       m_material_handle;

		glm::mat4 m_model_to_world_transform;
	};

}

#endif // !
