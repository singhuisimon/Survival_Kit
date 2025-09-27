#pragma once
#ifndef __SHARED_GRAPHICS_H__
#define __SHARED_GRAPHICS_H__
#include "../Graphics/Common.h"

namespace gam300 {
	namespace gfx {
		inline void bind_texture_and_sampler(GLuint unit, u64 texHandle, u64 sampleHandle) {
			glBindTextureUnit(unit, static_cast<GLuint>(texHandle));
			glBindSampler(unit, static_cast<GLuint>(sampleHandle));
		}
	}
}


#endif