/**
 * @file TextureComponent.cpp
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
#include "../Component/TextureComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {
	
	TextureComponent::TextureComponent(const std::string& guid, u16 texture_handle) : m_guid(guid), m_texture_handle(texture_handle) { }

	void TextureComponent::init(EntityID entity_id) {
		m_owner_id = entity_id;
		LM.writeLog("TextureComponent::init() - TextureComponent initialized for entity %d", entity_id);
	}

	void TextureComponent::update(float dt) {
		(void)dt;
	}

}