/**
 * @file RenderSystem.cpp
 * @brief Entity-component system for rendering scene geometry
 * @details Processes entities with Transform3D and MeshComponent to generate
 *          draw calls. Maintains a camera, lighting state, and builds a draw
 *          list each frame for submission to the renderer. Responsible for
 *          culling, sorting, and batching renderable entities.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "RenderSystem.h"
#include "../Component/ComponentView.h"
#include "../Graphics/SharedGraphics.h"
#include "../Graphics/ShaderProgram.h"
#include "../Manager/GraphicsManager.h"

namespace gam300 {

	RenderSystem::RenderSystem() : ComponentSystem<Transform3D, MeshComponent>("RenderSystem") {
		set_priority(151);
	}

	bool RenderSystem::init(SystemManager&) {
		LM.writeLog("RenderSystem::init() - Render System Initialized");

		m_draw_list.reserve(1000);
		m_camera = Camera3D(ORBITING, glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.f, 0.f, 0.0f), 45.0f, 0.5f, 100.0f);
		m_light = Light(glm::vec3(0.0f, 8.0f, 0.0f),
						glm::vec3(0.4f, 0.4f, 0.4f),
						glm::vec3(1.0f, 1.0f, 1.0f),
						glm::vec3(1.0f, 1.0f, 1.0f));

		return true;
	}

	void RenderSystem::update(float dt) {

		(void)dt;

		m_draw_list.clear();

		for (EntityID entity_id : m_entities) {
			process_entity(entity_id);
		}

		// Create a view of the draw item each update loop
		std::span<DrawItem> draw_span(m_draw_list.data(), m_draw_list.size());

		// Render each frame
		GFXM.getRenderer().render_frame(draw_span, m_camera, m_light);
		
	}

	void RenderSystem::shutdown() {
		LM.writeLog("RenderSystem::shutdown() - Render System shut down");
	}

	void RenderSystem::process_entity(EntityID entity_id) {

		Transform3D*     transform  = CM.get_component<Transform3D>(entity_id);
		MeshComponent*   mesh       = CM.get_component<MeshComponent>(entity_id);

		if (transform && mesh) {

			u16 mesh_handle     = mesh->getMeshHandle();
			u16 material_handle = mesh->getMaterialHandle();
			
			glm::mat4 model_to_world_transform = transform->getTransformationMatrix();

			// Add to draw list
			m_draw_list.push_back(DrawItem{ mesh_handle,0,material_handle,model_to_world_transform });
		}
	}

}