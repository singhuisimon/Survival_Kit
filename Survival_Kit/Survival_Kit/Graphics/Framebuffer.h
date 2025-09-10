#pragma once


#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
#include "../Glad/glad.h"

namespace gam300 {

	// RAII wrapper around framebuffer object
	class FrameBuffer {
	public:

		FrameBuffer() {

			// Generates a framebuffer, but the object is not initialized
			// requires binding before usage.
			glGenFramebuffers(1, &fbo_hdl);
		}

		~FrameBuffer() {
			// Automatically releases resources
			glDeleteFramebuffers(1, &fbo_hdl);
		}

		// Getter function
		unsigned int GetHandle() const { return fbo_hdl; }

	private:
		unsigned int fbo_hdl;
	};

}

#endif // !__FRAMEBUFFER_H__
