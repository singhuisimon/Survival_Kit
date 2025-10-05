/**
 * @file MeshComponent.h
 * @brief Declaration of the MeshComponent for the Entity Component System.
 * @details Handles the mesh and materials properties for entities.
 * @author	Chua Wen Bin Kenny 
 * @date	30/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "../Component/Component.h"
 //#include "../Utility/Vector3D.h"
#include "../Manager/LogManager.h"
#include <string>

namespace gam300 {

	class MeshComponent : public Component {

	private:
		std::string m_guid;			// Unique identifier for the audio resource
		uint16_t m_mesh_handle;		// Mesh handle
		uint16_t m_material_handle;	// Material handle

	public:

		// Constructor
		MeshComponent(const std::string& guid = "",
			uint16_t mesh_handle = 0,
			uint16_t material_handle = 0);

		// Initialization and Update
		void init(EntityID entity_id) override;
		void update(float dt) override;

		// Getters
		const std::string& getGUID() const { return m_guid; }
		const uint16_t& getMeshHandle() const { return m_mesh_handle; }
		const uint16_t& getMaterialHandle() const { return m_material_handle; }

		// Setters
		void setGUID(const std::string& guid) { m_guid = guid; }
		void setMeshHandle(const uint16_t& mesh_handle) { m_mesh_handle = mesh_handle; }
		void setMaterialHandle(const uint16_t& material_handle) { m_material_handle = material_handle; }

	};
} // namespace gam300

#endif // __MESHCOMPONENT_H__