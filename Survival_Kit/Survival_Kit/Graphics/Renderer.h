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
#include "../Utility/AssetPath.h"

#include "../Glad/glad.h"

namespace gam300 {

	class Renderer {

	public:

		void setup();
		void render_frame(std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light);

		inline GLuint get_imgui_texture() const {
			LM.writeLog("Renderer::get_imgui_texture this=%p\n", (void*)this); return d_imgui_texture;
		}

	private:

		void beginFrame();
		void draw(std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light);
		void endFrame();

		std::unordered_map<u32, MeshGL>			 m_tmp_mesh_storage;
		std::vector<ShaderProgram>               m_shader_storage;

		GLuint					                 d_imgui_texture = 0;
		std::optional<FrameBuffer>               d_fbo;
	};

}

#endif // !__RENDERER_H__
