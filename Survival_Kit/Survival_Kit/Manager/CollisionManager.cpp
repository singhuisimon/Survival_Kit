#include "CollisionManager.h"

namespace gam300
{
	AABB::AABB() : min{ 0.f, 0.f, 0.f }, max{ 0.f, 0.f, 0.f }
	{
	}

	AABB::AABB(std::vector<glm::vec3> const &pts)
	{
		for (auto const &pt : pts)
		{
			min.x = (min.x > pt.x) ? pt.x : min.x;
			min.y = (min.y > pt.y) ? pt.y : min.y;
			min.z = (min.z > pt.z) ? pt.z : min.z;

			max.x = (max.x < pt.x) ? pt.x : max.x;
			max.y = (max.y < pt.y) ? pt.y : max.y;
			max.z = (max.z < pt.z) ? pt.z : max.z;
		}
	}

	AABB::AABB(std::vector<glm::vec3> const &pts, glm::mat4 tfm)
	{
		std::vector<glm::vec3> newpts(pts.size());
		for (size_t i{}; i < pts.size(); ++i)
		{
			newpts[i] = tfm * glm::vec4(pts[i], 1.f);
		}
		*this = AABB(newpts);
	}

	AABB::AABB(AABB &other)
	{
		min = other.min;
		max = other.max;
	}

	AABB::AABB(AABB obj1, AABB obj2)
	{
		min.x = (obj1.min.x < obj2.min.x) ? obj1.min.x : obj2.min.x;
		min.y = (obj1.min.y < obj2.min.y) ? obj1.min.y : obj2.min.y;
		min.z = (obj1.min.z < obj2.min.z) ? obj1.min.z : obj2.min.z;

		max.x = (obj1.max.x > obj2.max.x) ? obj1.max.x : obj2.max.x;
		max.y = (obj1.max.y > obj2.max.y) ? obj1.max.y : obj2.max.y;
		max.z = (obj1.max.z > obj2.max.z) ? obj1.max.z : obj2.max.z;
	}

	glm::vec3 AABB::getCenter()
	{
		return (max + min) / 2.f;
	}

	glm::vec3 AABB::getExtents()
	{
		return max - min;
	}

	bool AABB::intersects(AABB const &other) const
	{
		return (min.x <= other.max.x && max.x >= other.min.x &&
			min.y <= other.max.y && max.y >= other.min.y &&
			min.z <= other.max.z && max.z >= other.min.z);
	}

}