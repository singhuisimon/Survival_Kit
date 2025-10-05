/**
 * @file RenderComponent.h
 * @brief Entity component for renderable mesh assignment
 * @details Component that marks an entity as renderable and associates it
 *          with a mesh resource. Stores a handle for efficient mesh lookups
 *          in the renderer. Part of the entity-component system for managing
 *          visual geometry.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __RENDER_COMPONENT_H__
#define __RENDER_COMPONENT_H__

#include "../Component/Component.h"
#include "../Graphics/SharedGraphics.h"

namespace gam300 {

	/**
	 * @brief Component that assigns renderable geometry to an entity
	 * @details Binds a mesh resource to an entity, making it visible in the scene.
	 *          The mesh handle indexes into the renderer's mesh storage for fast
	 *          lookups during rendering. Entities with this component participate
	 *          in the draw call submission process.
	 */
	class RenderComponent : public Component {
	public:
		/**
		 * @brief Initializes the component when attached to an entity
		 * @param entity_id The entity this component is attached to
		 */
		void init(EntityID entity_id) override;

		/**
		 * @brief Updates the component each frame
		 * @param dt Delta time in seconds since last update
		 */
		void update(float dt) override;

		/**
		 * @brief Gets the runtime mesh handle for renderer lookups
		 * @return Mesh handle (index into renderer's mesh storage)
		 */
		inline u32 get_mesh_handle() const { return m_mesh_handle; }

	private:
		u32 m_mesh_handle = 0;
	};

}


#endif