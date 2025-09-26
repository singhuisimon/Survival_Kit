#pragma once

#ifndef __MESH_DATA_H__
#define __MESH_DATA_H__

// For dynamically sized arrays
#include <vector>

// For access to vector types
#include <glm-0.9.9.8/glm/glm.hpp>

// For access to VAO and VBO wrappers
#include "../Graphics/GLResources.h"

namespace gam300 {
	
	// Helpful container to store per mesh data, extendable
	struct MeshData {

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec2> texcoords;
		std::vector<uint32_t>  indices;
	};

	struct MeshGL {

		VAO vao{};
		VBO vbo{};
		VBO ebo{};

		GLsizei draw_count = 0;

		GLenum  primitive_type = GL_TRIANGLES;
		GLenum  index_type     = GL_UNSIGNED_INT;

		MeshGL(const MeshGL&) = delete;
		MeshGL& operator=(const MeshGL&) = delete;
		MeshGL() = default;
		~MeshGL() = default;
		MeshGL(MeshGL&&) noexcept = default;
		MeshGL& operator=(MeshGL&&) noexcept = default;
	};
	
}

#endif