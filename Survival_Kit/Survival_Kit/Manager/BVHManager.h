#pragma once

#include <glm-0.9.9.8/glm/glm.hpp>
#include <vector>
#include <array>

namespace gam300 {
	struct AABB {
		glm::vec3 min;
		glm::vec3 max;

		// Default
		AABB();
		// Single Arg (Vector of points)
		AABB(std::vector<glm::vec3> const& pts);
		// Single Arg (Vector of points + Transform)
		AABB(std::vector<glm::vec3> const& pts, glm::mat4 tfm);
		// Copy Constructor
		AABB(AABB& other);
		// Expanded AABB
		AABB(AABB& obj1, AABB& obj2);

		glm::vec3 extents();
	};
}