/**
 * @file GLResources.h
 * @brief RAII Wrappers for GPU resources (OpenGL based).
 * @details Contains the prototype of RAII wrappers for GPU resources like vertex array objects and buffer objects.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __GLRESOURCES_H__
#define __GLRESOURCES_H__

// For OpenGL functionality
#include "../Glad/glad.h"

namespace gam300 {
	
	/**
	 * @brief RAII wrapper for OpenGL buffer object
	 * @details Wraps around an OpenGL buffer object.
	 */
	class VBO {

	public:
		/**
		 * @brief Constructor for buffer.
		 */
		VBO();

		/**
		 * @brief Destructor for buffer.
		 */
		~VBO();

		// Prevent copy construction and copy assignment
		VBO(const VBO&) = delete;
		VBO& operator=(const VBO&) = delete;

		/**
		 * @brief Move constructor for buffer.
		 */
		VBO(VBO&&) noexcept;

		/**
		 * @brief Move assignment operator overload for buffer.
		 */
		VBO& operator=(VBO&&) noexcept;

		/**
		 * @brief Creates and binds a buffer for GPU operations.
		 * @details Deletes any existing buffer, before calling glCreateBuffers
		 */
		void create();

		/**
		 * @brief Gets the underlying handle to the buffer object.
		 */
		GLuint id() const noexcept;

		/**
		 * @brief Initialize buffer object's data store.
		 * @details Initialize the buffer object's immutable data store, NOT the buffer object itself.
		 * Calls glNamedBufferStorage to achieve this.
		 */
		void storage(GLsizeiptr size, const void* data, GLbitfield flags);

		/**
		 * @brief Updates a subset of a buffer object's data store
		 * @details Calls glNamedBufferSubData to achieve this.
		 */
		void sub_data(GLintptr offset, GLsizeiptr size, const void* data);

	private:
	   /**
		* @brief Helper function in releasing resrouces
		*/	
		void destroy();

		// Underlying data store
		GLuint handle;

	};

	/**
	 * @brief RAII wrapper for OpenGL vertex array object.
	 * @details Wraps around an OpenGL vertex array object.
	 */
	class VAO {

	public:
		/**
		 * @brief Constructor for vertex array object.
		 */
		VAO();

		/**
		 * @brief Destructor for vertex array object.
		 */
		~VAO();

		// Disable copy construction and copy assignment
		VAO(const VAO&) = delete;
		VAO& operator=(const VAO&) = delete;

		/**
		 * @brief Move constructor for vertex array object.
		 */
		VAO(VAO&&) noexcept;

		/**
		 * @brief Move assignment operator overload for vertex array object.
		 */
		VAO& operator=(VAO&&) noexcept;

		/**
		 * @brief Creates and binds a vertex array object for GPU operations.
		 * @details Deletes any existing vertex array object, before calling glCreateVertexArrays
		 */
		void create();

		/**
		 * @brief Gets the underlying handle to the vertex array object.
		 */
		GLuint id() const noexcept;

		/**
		 * @brief Binds this vertex array for GPU operations (e.g. draw calls)
		 */
		void bind() const;
		static void unbind();

		/**
		 * @brief Enables vertex array attributes via direct state access
		 * @details Calls glEnableVertexArrayAttrib to achieve this.
		 */
		void enable_attrib(GLuint attrib) const;

		void bind_vertex_buffer(GLuint brinding, const VBO& buf, GLintptr offset, GLsizei stride) const;
		
		void attrib_format(GLuint attrib, GLint comps, GLenum type, bool normalized, GLuint relativeOffset) const;

		void attrib_binding(GLuint attrib, GLuint binding) const;

		void bind_element_buffer(const VBO& ebo) const;

	private:
		/**
		 * @brief Helper function in releasing resrouces
		 */
		void destroy();

		// Underlying handle to the vertex array object
		GLuint handle;
	};
}

#endif // !__GLRESOURCES_H__
