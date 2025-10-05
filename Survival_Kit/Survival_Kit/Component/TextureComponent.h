/**
 * @file TextureComponent.h
 * @brief Entity component for texture assignment
 * @details Component that associates a texture resource with an entity.
 *          Stores both a GUID for serialization/identification and a runtime
 *          handle for efficient renderer lookups. Part of the entity-component
 *          system for managing visual appearance.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __TEXTURE_COMPONENT_H__
#define __TEXTURE_COMPONENT_H__

#include "../Component/Component.h"
#include "../Graphics/SharedGraphics.h"

namespace gam300 {

	/**
	 * @brief Component that assigns a texture to an entity
	 * @details Binds a texture resource to an entity for rendering. Uses dual
	 *          identification: GUID for persistence/asset management and handle
	 *          for runtime performance. The handle indexes into the renderer's
	 *          texture storage for efficient lookups during draw calls.
	 */
	class TextureComponent : public Component {
	public:
		/**
		 * @brief Constructs a texture component with optional initial values
		 * @param guid Globally unique identifier for the texture asset
		 * @param texture_handle Index into renderer's texture storage (default: 0)
		 */
		TextureComponent(const std::string& guid = "", u16 texture_handle = 0);

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
		 * @brief Gets the texture's globally unique identifier
		 * @return Const reference to the GUID string
		 */
		inline const std::string& getGUID() const { return m_guid; }

		/**
		 * @brief Gets the runtime texture handle for renderer lookups
		 * @return Const reference to the texture handle (index into storage)
		 */
		inline const uint16_t& getTextureHandle() const { return m_texture_handle; }

		/**
		 * @brief Sets the texture's globally unique identifier
		 * @param guid New GUID for the texture asset
		 */
		inline void setGUID(const std::string& guid) { m_guid = guid; }

		/**
		 * @brief Sets the runtime texture handle
		 * @param tex_hdl New texture handle (index into renderer's texture storage)
		 */
		inline void setTextureHandle(u16& tex_hdl) { m_texture_handle = tex_hdl; }

	private:

		std::string m_guid;
		u16         m_texture_handle;
	};

}

#endif // !__TEXTURE_COMPONENT_H__
