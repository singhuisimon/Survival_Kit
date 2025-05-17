/**
 * @file GLRenderer.h
 * @brief Declares the interface of a renderer that uses OpenGL as it's graphics backend.
 * @details This file provides the class declaration for the OpenGL-based renderer,
 * including methods that interface with the OpenGL API to execute rendering operations.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#ifndef __GL_RENDERER_H__
#define __GL_RENDERER_H__

#include <vector>
#include "GLFW/glfw3.h"
#include "IRenderer.h"
#include "RenderCommand.h"

namespace gam300 {

	class GLRenderer : public IRenderer {
	public:
		void init() override;
		void shutdown() override;
		void beginFrame() override;
		void endFrame() override;
		void submit() override;

	private:
		GLFWwindow* m_window = nullptr;
		std::vector<RenderCommand> m_command_queue;
	};

}

#endif // !__GL_RENDERER_H__
