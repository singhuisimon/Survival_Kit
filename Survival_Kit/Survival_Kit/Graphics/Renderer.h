#pragma once

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <vector>
#include <unordered_map>
#include <cstdio>

#include "../Graphics/Light.h"
#include "../Graphics/Camera.h"
#include "../Graphics/DrawItem.h"
#include "../Graphics/Shape.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Material.h"

#include "../Utility/AssetPath.h"

#include "../Glad/glad.h"

namespace gam300 {

	class Renderer {

	public:

		void setup();
		void render_frame(std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light);

		inline GLuint get_imgui_texture() const { return static_cast<GLuint>(m_textures[0].handle()); }
		inline const size_t mesh_count() const { return m_mesh_storage.size(); }
		inline const std::vector<MeshData>& getMeshDataStorage() { return m_mesh_data_storage; }
		inline const std::vector<Material>& getMaterialStorage() { return t_testing_material;  }
		inline const std::vector<Texture>& getTextureStorage()   { return t_testing_textures; }
	private:

		void beginFrame(RenderPass const& pass);
		void draw(RenderPass const& pass, std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light);
		void endFrame(RenderPass const& pass);

		std::vector<MeshGL>                      m_mesh_storage;
		std::vector<MeshData>                    m_mesh_data_storage;
		std::vector<ShaderProgram>               m_shader_storage;
		std::vector<RenderPass>                  m_passes;
		std::vector<FrameBuffer>                 m_framebuffers;
		std::vector<Texture>                     m_textures;

		std::vector<Texture>                     t_testing_textures;
		std::vector<Material>                    t_testing_material;
	};

}

#endif // !__RENDERER_H__
