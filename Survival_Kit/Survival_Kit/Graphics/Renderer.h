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

	private:

		void beginFrame(RenderPass const& pass);
		void draw(RenderPass const& pass, std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light);
		void endFrame(RenderPass const& pass);

		std::unordered_map<u32, MeshGL>			 m_tmp_mesh_storage;

		std::vector<ShaderProgram>               m_shader_storage;
		std::vector<RenderPass>                  m_passes;
		std::vector<FrameBuffer>                 m_framebuffers;
		std::vector<Texture>                     m_textures;

		Material                                 test_material;
	};

}

#endif // !__RENDERER_H__
