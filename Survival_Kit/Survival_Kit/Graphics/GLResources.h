// contains wrappers for GPU resources such as vertex array objects and vertex buffer objects

#pragma once

#ifndef __GLRESOURCES_H__
#define __GLRESOURCES_H__
#include "../Glad/glad.h"

namespace gam300 {
	
	// RAII wrapper for buffer object
	class VBO {

	public:

		// Creates a buffer object and binds to an unspecified target
		VBO();

		// Automatically releases GPU resources when object is destroyed
		~VBO();

		// Prevent copy construction and copy assignment
		VBO(const VBO&) = delete;
		VBO& operator=(const VBO&) = delete;

		// Move construction
		VBO(VBO&&) noexcept;

		// Move assignment
		VBO& operator=(VBO&&) noexcept;

		void create();

		// Gets the underlying handle to this object
		GLuint id() const noexcept;

		// Initialize the buffer object's immutable data store, NOT the buffer object itself
		void storage(GLsizeiptr size, const void* data, GLbitfield flags);

		// Updates a subset of a buffer object's data store
		void sub_data(GLintptr offset, GLsizeiptr size, const void* data);

	private:

		// Underlying data store
		GLuint handle;

		// Helper function in releasing resrouces
		void destroy();
	};

	class VAO {

	public:

		VAO();
		~VAO();

		// Disable copy construction and copy assignment
		VAO(const VAO&) = delete;
		VAO& operator=(const VAO&) = delete;

		// Move construction and assignment
		VAO(VAO&&) noexcept;
		VAO& operator=(VAO&&) noexcept;

		void create();

		// Getter
		GLuint id() const noexcept;

		// Binds this vertex array for GPU operations (e.g. draw calls)
		void bind() const;
		static void unbind();

		// Enable vertex array attrib DSA
		void enable_attrib(GLuint attrib) const;

		void bind_vertex_buffer(GLuint brinding, const VBO& buf, GLintptr offset, GLsizei stride) const;
		
		void attrib_format(GLuint attrib, GLint comps, GLenum type, bool normalized, GLuint relativeOffset) const;

		void attrib_binding(GLuint attrib, GLuint binding) const;

		void bind_element_buffer(const VBO& ebo) const;

	private:

		GLuint handle;

		// Helper function in releasing resrouces
		void destroy();
	};

}

#endif // !__GLRESOURCES_H__
