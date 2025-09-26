#pragma once

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <optional>
#include <array>
#include <span>

#include "../Graphics/Common.h"

namespace gam300 {

	// RAII wrapper around framebuffer object
	class FrameBuffer {
	public:
		FrameBuffer() = delete;

		inline static std::optional<FrameBuffer> create() {
			GLuint fbo = kInvalid;
			glCreateFramebuffers(1, &fbo);

			if (fbo == 0) {
				return std::nullopt;
			}

			return FrameBuffer(static_cast<u64>(fbo));
		}

		FrameBuffer(FrameBuffer&& o) noexcept { move_from(o); }
		FrameBuffer& operator=(FrameBuffer&& o) noexcept {
			if (this != &o) { destroy(); move_from(o); }
			return *this;
		}

		~FrameBuffer() noexcept { destroy(); }

		u64  handle() const noexcept { return m_handle; }
		bool valid() const noexcept { return m_handle != kInvalid; }

		inline void attach_color(GLenum attachment, GLuint tex, GLint level = 0) const {
			glNamedFramebufferTexture(gl_id(), attachment, tex, level);
		}

		inline void attach_renderbuffer(GLenum attachment, GLuint rbo) const {
			glNamedFramebufferRenderbuffer(gl_id(), attachment, GL_RENDERBUFFER, rbo);
		}

		inline void attach_depth(GLuint texOrRb, bool isTexture, GLint level = 0) const {
			if (isTexture) glNamedFramebufferTexture(gl_id(), GL_DEPTH_ATTACHMENT, texOrRb, level);
			else glNamedFramebufferTexture(gl_id(), GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texOrRb);
		}

		inline void attach_depth_stencil(GLuint texOrRb, bool isTexture, GLint level = 0) const {
			if (isTexture) glNamedFramebufferTexture(gl_id(), GL_DEPTH_STENCIL_ATTACHMENT, texOrRb, level);
			else glNamedFramebufferRenderbuffer(gl_id(), GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, level);
		}

		inline void set_draw_buffers(std::span<const GLenum> bufs) const {
			glNamedFramebufferDrawBuffers(gl_id(), static_cast<GLsizei>(bufs.size()), bufs.data());
		}

		inline void set_read_buffer(GLenum buf) const { glNamedFramebufferReadBuffer(gl_id(), buf); }

		inline bool complete() const {
			return glCheckNamedFramebufferStatus(gl_id(), GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		inline void clear_colori(GLint drawbuf, int r, int g, int b, int a) const {
			glClearNamedFramebufferiv(gl_id(), GL_COLOR, drawbuf, std::array<GLint, 4>{r,g,b,a}.data());
		}

		inline void clear_colorf(GLint drawbuf, float r, float g, float b, float a) const {
			glClearNamedFramebufferfv(gl_id(), GL_COLOR, drawbuf, std::array<float, 4>{r, g, b, a}.data());
		}

		inline void clear_depth(float d) const { glClearNamedFramebufferfv(gl_id(), GL_DEPTH, 0, &d); }
		inline void clear_stencil(GLint s) const { glClearNamedFramebufferiv(gl_id(), GL_STENCIL, 0, &s); }

		static void blit(const FrameBuffer& src, const FrameBuffer& dst, 
						 GLint sx0, GLint sy0, GLint sx1, GLint sy1, 
						 GLint dx0, GLint dy0, GLint dx1, GLint dy1, 
						 GLbitfield mask, GLenum filter) {

			glBlitNamedFramebuffer(src.gl_id(), dst.gl_id(), sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
		}

		inline void invalidate(std::span<const GLenum> attachments) const {
			glInvalidateNamedFramebufferData(gl_id(), static_cast<GLsizei>(attachments.size()), attachments.data());
		}

		// Readback
		inline void read_pixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* dst) const {
			// must have set a read buffer before this if color read.
			glReadnPixels(x, y, w, h, format, type, w * h * 4, dst);
		}

		// debug label
		void label(const char* name) const {
			glObjectLabel(GL_FRAMEBUFFER, gl_id(), -1, name);
		}
	 
	private:
		explicit FrameBuffer(u64 h) noexcept : m_handle(h) { }

		void destroy() noexcept {
			if (m_handle != kInvalid) { const GLuint fb = static_cast<GLuint>(m_handle); glDeleteFramebuffers(1, &fb); }
		}

		void move_from(FrameBuffer& o) {
			m_handle = o.m_handle; o.m_handle = kInvalid;
		}

		GLuint gl_id() const { return static_cast<GLuint>(m_handle); }

		static constexpr u64 kInvalid = 0;
		u64 m_handle = kInvalid;
	};

}

#endif // !__FRAMEBUFFER_H__
