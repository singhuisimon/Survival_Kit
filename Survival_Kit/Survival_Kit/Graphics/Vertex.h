#pragma once

#ifndef __VERTEX_H__
#define __VERTEX_H__
#include <vector>
#include <glm-0.9.9.8/glm/glm.hpp>

namespace gam300 {

	// Helpful container to store per mesh data, extendable
	struct VertexSoA {

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		
	};

}

#endif