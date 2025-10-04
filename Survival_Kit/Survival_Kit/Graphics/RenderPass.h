#pragma once
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include <span>

#include "../Graphics/DrawItem.h"
#include "../Graphics/ShaderProgram.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Light.h"

namespace gam300 {
	
	enum class PassType : uint8_t {GEOMETRY, FULLSCREEN, DEBUG};

	struct RenderPass {
		std::string     pass_name;
		u32             fbo_handle; 
		u32             shdpgm_handle;
		glm::vec4       clear_color_value = { 1, 1, 1, 1 };
		glm::vec4       view_port = { 0, 0, 640, 480 };
		bool            clear_color = true;
		bool            clear_depth = true;
		bool            depth_test  = true;
		bool            depth_write = true;
		bool            blending    = false;
		bool            culling     = true;
		PassType        passtype    = PassType::GEOMETRY;
	};
}

#endif // !__RENDER_PASS_H__
