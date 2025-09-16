#pragma once

#ifndef __MESH_DATA_H__
#define __MESH_DATA_H__

// For GLushort type and declaring primitive type
#include "../Glad/glad.h"

// For dynamically sized arrays
#include <vector>

// For access to vector types
#include <glm-0.9.9.8/glm/glm.hpp>

namespace gam300 {

	// Helpful container to store per mesh data, extendable
	struct MeshData {

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec2> texcoords;
		std::vector<GLushort>  indices;

		GLenum primitive_type;
	};

}

#endif