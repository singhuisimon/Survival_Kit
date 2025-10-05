/**
 * @file SharedGraphics.h
 * @brief Common graphics utility functions for OpenGL operations
 * @details Provides helper functions and convenience wrappers for frequently
 *          used OpenGL 4.6 operations, simplifying common rendering tasks
 *          and reducing code duplication across the rendering system.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#ifndef __SHARED_GRAPHICS_H__
#define __SHARED_GRAPHICS_H__
#include "../Graphics/Common.h"

namespace gam300 {
	namespace gfx {

		/**
		 * @brief Binds a texture and sampler to a specified texture unit
		 * @param unit The texture unit index to bind to (0-based)
		 * @param texHandle OpenGL texture handle (64-bit bindless or regular handle)
		 * @param sampleHandle OpenGL sampler object handle
		 */
		inline void bind_texture_and_sampler(GLuint unit, u64 texHandle, u64 sampleHandle) {
			glBindTextureUnit(unit, static_cast<GLuint>(texHandle));
			glBindSampler(unit, static_cast<GLuint>(sampleHandle));
		}
	}
}


#endif