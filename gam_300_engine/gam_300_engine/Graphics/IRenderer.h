/**
 * @file IRenderer.h
 * @brief Base interface for renderer in the graphics system.
 * @details Defines the abstract renderer class that wraps around a low-level graphics API.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __I_RENDERER_H__
#define __I_RENDERER_H__

namespace gam300 {

	/**
	 * @brief Represents the underlying API used by the renderer
	 * @details Provides renderer different API options when creating an instance of the renderer
	 */
	enum class GFXAPI { OpenGL, Vulkan };

	class IRenderer {
	public:

		virtual ~IRenderer()       = default;

		// Initialize the system
		virtual void init()		   = 0;

		// Shutsdown the system, and cleanup after use
		virtual void shutdown()    = 0;

		// Preps the beginning of a frame
		virtual void beginFrame()  = 0;

		// Cleanup at the end of a frame
		virtual void endFrame()	   = 0;

		// Submit render calls
		virtual void submit()	   = 0;

		// Checks if the window should close
		virtual bool shouldClose() = 0;
	};
}

#endif // !__RENDERER_H__

