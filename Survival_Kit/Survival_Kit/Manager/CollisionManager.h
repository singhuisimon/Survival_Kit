#pragma once

#include <glm-0.9.9.8/glm/glm.hpp>
#include <vector>
#include <array>
#include <memory>

#include "ComponentManager.h">
#include "ECSManager.h"

namespace gam300
{
	//static BVHManager bvh;

	struct AABB
	{
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

		glm::vec3 min;
		glm::vec3 max;
	};

	//struct BVHConfig
	//{
	//	unsigned maxDepth = std::numeric_limits<unsigned>::max();
	//	unsigned minObjects = 1;
	//	float minVolume = 0;
	//};

	//struct BVHWrapper
	//{
	//	EntityID	id{};
	//	AABB		bv{};

	//	struct
	//	{
	//		BVHWrapper *next{ nullptr };
	//		BVHWrapper *prev{ nullptr };
	//		void *node{ nullptr };
	//	} bvhInfo;
	//};

	//class BVHManager
	//{
	//public: // Public for testing reasons
	//	struct Node
	//	{
	//		AABB  bv;          // Node bounding volume
	//		Node *children[2]; // Both children
	//		BVHWrapper     firstObject; //
	//		BVHWrapper     lastObject;  //

	//		void                        AddObject(BVHWrapper object);
	//		int                         Depth() const; // Branches to leaves
	//		int                         Size() const;  // Amount of nodes
	//		bool                        IsLeaf() const;// Checks if leaf node
	//		unsigned                    ObjectCount() const; // Self-explanatory
	//	};

	//private:
	//	Node *mRoot;
	//	unsigned mObjectCount;

	//public:
	//	BVHManager();
	//	~BVHManager();
	//	BVHManager(BVHManager const &) = delete;
	//	BVHManager &operator=(BVHManager const &) = delete;

	//	template <typename IT> void Build(IT begin, IT end, BVHConfig const &config);
	//	template <typename IT> void Insert(IT begin, IT end, BVHConfig const &config);
	//	void                        Insert(T object, BVHConfig const &config);
	//	void                        Clear();
	//	bool                        Empty() const;
	//	int                         Depth() const;
	//	int                         Size() const;

	//	Node const *root() const;

	//	//std::vector<unsigned> Query(Frustum const &frustum) const;

	//	unsigned objectCount() const
	//	{
	//		return mObjectCount;
	//	}

	//private:
	//	void SplitNode(Node *node, BVHConfig const &config, unsigned depth);
	//	Node *InsertSubtree(Node *node, Node *subtree, BVHConfig const &config);
	//};

	static Octree OTManager;

	struct ObjNode
	{
		unsigned id{};
		std::unique_ptr<ObjNode> next{ nullptr };
	};

	struct OctNode
	{
		std::array<std::unique_ptr<OctNode>, 8> child{};
		std::unique_ptr<ObjNode> head{ nullptr };
	};

	class Octree
	{
	public:
		void setWorld(AABB w)
		{
			world = w;
		}

		void insert(EntityID id)
		{
			auto const &e = EM.getEntity(id);
		}

		void remove(EntityID id)
		{
			auto const &e = EM.getEntity(id);
		}

		std::vector<EntityID> retrieveEntitiesInNode(EntityID id)
		{

		}

		AABB world{};
		std::unique_ptr<OctNode> root{};
		
		// std::unordered_map<unsigned, OctNode*> 4+8bytes/entry
		// std::vector<unsigned> paths; 4/entry
		
	};
}