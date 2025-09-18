#pragma once

#include <glm-0.9.9.8/glm/glm.hpp>
#include <vector>
#include <array>
#include <memory>

#include "ComponentManager.h"
#include "ECSManager.h"

#define OT gam300::Octree::getInstance()

namespace gam300 {
	struct AABB {
		// Default
		AABB();
		// Single Arg (Vector of points)
		AABB(std::vector<glm::vec3> const &pts);
		// Single Arg (Vector of points + Transform)
		AABB(std::vector<glm::vec3> const &pts, glm::mat4 tfm);
		// Copy Constructor
		AABB(AABB &other);
		// Expanded AABB
		AABB(AABB obj1, AABB obj2);

		glm::vec3 getCenter();
		glm::vec3 getExtents();

		bool intersects(AABB const &other) const;
		bool contains(AABB const &other) const;

		glm::vec3 min;
		glm::vec3 max;
	};

	struct ObjNode {
		unsigned id{};
		std::unique_ptr<ObjNode> next{ nullptr };
	};

	struct OctNode {
		std::array<std::unique_ptr<OctNode>, 8> child{};
		std::unique_ptr<ObjNode> head{ nullptr };
	};

	struct OctConfig {
		unsigned max_depth{ 7 };
		unsigned min_objects{ 4 };
	} cfg;

	class Octree {
	public:
		static Octree &getInstance();
		void setWorld(AABB w);
		void insert(EntityID id);
		void insert(std::vector<EntityID> ids);
		void remove(EntityID id);
		std::unique_ptr<OctNode> &traversePath(unsigned p);
		unsigned traverseBestPath(AABB &obj);

		//std::vector<EntityID> retrieveVisible(Frustum const &f);

		AABB world{};
		std::unique_ptr<OctNode> root{};
		std::vector<unsigned> path{};
	};
}