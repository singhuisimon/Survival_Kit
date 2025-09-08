#include "BVHManager.h"

namespace gam300 {
	AABB::AABB() : min({}), max({}) {}

	AABB::AABB(std::vector<glm::vec3> const& pts) {
		for (auto const& pt : pts) {
			min.x = (min.x > pt.x) ? pt.x : min.x;
			min.y = (min.y > pt.y) ? pt.y : min.y;
			min.z = (min.z > pt.z) ? pt.z : min.z;

			max.x = (max.x < pt.x) ? pt.x : max.x;
			max.y = (max.y < pt.y) ? pt.y : max.y;
			max.z = (max.z < pt.z) ? pt.z : max.z;
		}
	}

	AABB::AABB(std::vector<glm::vec3> const& pts, glm::mat4 tfm) {
		std::vector<glm::vec3> newpts(pts.size());
		for (size_t i{}; i < pts.size(); ++i) {
			newpts[i] = glm::mat3(tfm) * pts[i];
		}
		*this = AABB(newpts);
	}
}