/**
 * @file MeshComponent.cpp
 * @brief Implementation of the MeshComponent for the Entity Component System.
 * @details Contains implementations for all member functions declared in MeshComponent.h.
 * @author	Chua Wen Bin Kenny 
 * @date	30/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Component/MeshComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {

	MeshComponent::MeshComponent(const std::string& guid,
		uint16_t mesh_handle,
		uint16_t material_handle) :

		m_guid(guid),
		m_mesh_handle(mesh_handle),
		m_material_handle(material_handle) {
	}

	// Initialize the component
	void MeshComponent::init(EntityID entity_id) {
		m_owner_id = entity_id;
		LM.writeLog("MeshComponent::init() - MeshComponent initialized for entity %d", entity_id);
	}
	// Update the component
	void MeshComponent::update(float dt) {
		// MeshComponent doesn't need to do much in update - it's primarily a data container
		// Render System will read and modify this data
		// Mark parameter as unused to avoid compiler warning
		(void)dt;
	}
}