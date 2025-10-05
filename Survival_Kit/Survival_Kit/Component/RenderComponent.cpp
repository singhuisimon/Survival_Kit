/**
 * @file RenderComponent.cpp
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
#include "RenderComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {

	void RenderComponent::init(EntityID entity_id) {
		m_owner_id = entity_id;
		LM.writeLog("RenderComponent::init() - RenderComponent component initialized for entity %d", entity_id);
	}

	void RenderComponent::update(float dt) {
		(void)dt;
		return;
	}

}