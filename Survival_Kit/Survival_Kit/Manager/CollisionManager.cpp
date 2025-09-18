#include "CollisionManager.h"

namespace gam300 {
	AABB::AABB() : min{ 0.f, 0.f, 0.f }, max{ 0.f, 0.f, 0.f } {}

	AABB::AABB(std::vector<glm::vec3> const &pts) {
		for(auto const &pt : pts) {
			min.x = (min.x > pt.x) ? pt.x : min.x;
			min.y = (min.y > pt.y) ? pt.y : min.y;
			min.z = (min.z > pt.z) ? pt.z : min.z;

			max.x = (max.x < pt.x) ? pt.x : max.x;
			max.y = (max.y < pt.y) ? pt.y : max.y;
			max.z = (max.z < pt.z) ? pt.z : max.z;
		}
	}

	AABB::AABB(std::vector<glm::vec3> const &pts, glm::mat4 tfm) {
		std::vector<glm::vec3> newpts(pts.size());
		for(size_t i{}; i < pts.size(); ++i) {
			newpts[i] = tfm * glm::vec4(pts[i], 1.f);
		}
		*this = AABB(newpts);
	}

	AABB::AABB(AABB &other) {
		min = other.min;
		max = other.max;
	}

	AABB::AABB(AABB obj1, AABB obj2) {
		min.x = (obj1.min.x < obj2.min.x) ? obj1.min.x : obj2.min.x;
		min.y = (obj1.min.y < obj2.min.y) ? obj1.min.y : obj2.min.y;
		min.z = (obj1.min.z < obj2.min.z) ? obj1.min.z : obj2.min.z;

		max.x = (obj1.max.x > obj2.max.x) ? obj1.max.x : obj2.max.x;
		max.y = (obj1.max.y > obj2.max.y) ? obj1.max.y : obj2.max.y;
		max.z = (obj1.max.z > obj2.max.z) ? obj1.max.z : obj2.max.z;
	}

	glm::vec3 AABB::getCenter() {
		return (max + min) / 2.f;
	}

	glm::vec3 AABB::getExtents() {
		return max - min;
	}

	bool AABB::intersects(AABB const &other) const {
		return (min.x <= other.max.x && max.x >= other.min.x &&
				min.y <= other.max.y && max.y >= other.min.y &&
				min.z <= other.max.z && max.z >= other.min.z);
	}

	bool AABB::contains(AABB const &other) const {
		return (min.x >= other.min.x && max.x <= other.max.x) &&
			(min.y >= other.min.y && max.y <= other.max.y) &&
			(min.z >= other.min.z && max.z <= other.max.z);
	}

	// Get the singleton instance
	Octree &Octree::getInstance() {
		static Octree instance;
		return instance;
	}

	void Octree::setWorld(AABB w) {
		world = w;
	}

	void Octree::insert(EntityID id) {
		if(!root) root = std::make_unique<OctNode>();
		if(id >= path.size()) path.resize(id + 1u, 0u);

		AABB obj{};
		// TODO: fill 'obj' from your ECS/components, e.g.:
		// obj = computeWorldAABBFromComponents(EM, CM, id);

		unsigned const packed = traverseBestPath(obj);
		path[id] = packed;

		OctNode *cur = root.get();

		if(packed == 0u) {
			auto n = std::make_unique<ObjNode>();
			n->id = id;
			n->next = std::move(cur->head);
			cur->head = std::move(n);
			return;
		}

		unsigned depth = 0u;
		while(true) {
			unsigned const shift = depth * 4u;
			unsigned const nib = (packed >> shift) & 0xFu;
			unsigned const cont = (nib >> 3u) & 1u;
			unsigned const oct = nib & 7u;

			if(cont == 1u) {
				if(!cur->child[oct]) cur->child[oct] = std::make_unique<OctNode>();
				cur = cur->child[oct].get();
				++depth;
				continue;
			}
			else {
				auto n = std::make_unique<ObjNode>();
				n->id = id;
				n->next = std::move(cur->head);
				cur->head = std::move(n);
				return;
			}
		}
	}

	void Octree::insert(std::vector<EntityID> ids) {
		for(auto i : ids) {
			OT.insert(i);
		}
	}

	void Octree::remove(EntityID id) {
		// Guard: id in bounds; also need a root to do anything useful.
		if(id >= path.size() || !root) {
			return;
		}

		unsigned &p = path[id];

		// Find terminal node for this object's path.
		std::unique_ptr<OctNode> &nodePtr = traversePath(p);
		OctNode *node = nodePtr.get();
		if(!node) {
			// Path points to a non-existent slot — nothing to unlink.
			p = 0u;
			return;
		}

		// Unlink from the singly-linked ObjNode list (unique_ptr chain).
		std::unique_ptr<ObjNode> *link = &node->head;
		while(link->get() && (*link)->id != id) {
			link = &((*link)->next);
		}

		if(link->get()) {
			// Remove this ObjNode by skipping it.
			*link = std::move((*link)->next);
		}
		else {
			// Not found at the expected node; clear path entry and bail.
			p = 0u;
			return;
		}

		// Optional light pruning: if the terminal node is now empty (no objects, no children),
		// delete this node. (Parents are not pruned here; would require a parent stack.)
		if(node->head == nullptr) {
			bool hasChild = false;
			for(int i = 0; i < 8; ++i) {
				if(node->child[i]) {
					hasChild = true;
					break;
				}
			}
			if(!hasChild) {
				nodePtr.reset();
			}
		}

		// Clear stored path for this entity.
		p = 0u;
	}

	std::unique_ptr<OctNode> &Octree::traversePath(unsigned p) {
		std::unique_ptr<OctNode> *cur = &root;

		// Terminal at root (object doesn't fit world / stopped at depth 0)
		if(p == 0u) {
			return *cur;
		}

		unsigned depth = 0u;
		while(true) {
			unsigned const shift = depth * 4u;
			unsigned const nib = (p >> shift) & 0xFu;
			unsigned const cont = (nib >> 3u) & 1u;
			unsigned const oct = nib & 7u;

			// If the current node doesn't exist, we can't go deeper - return this slot.
			if(!(*cur)) {
				return *cur;
			}

			// Descend to the child designated by this nibble.
			cur = &((*cur)->child[oct]);

			// Terminal nibble => stop after this descent.
			if(cont == 0u) {
				return *cur;
			}

			++depth;
		}
	}

	unsigned Octree::traverseBestPath(AABB &obj) {
		if(!world.contains(obj)) {
			return 0u;
		}

		unsigned packed{ 0u };
		unsigned depth{ 0u };
		AABB nodeBox{ world };

		while(true) {
			if(depth >= cfg.max_depth) {
				if(depth > 0u) {
					unsigned const pshift{ (depth - 1u) * 4u };
					unsigned pnib{ (packed >> pshift) & 0xFu };
					pnib &= ~0x8u;
					packed = (packed & ~(0xFu << pshift)) | (pnib << pshift);
				}
				else {
					packed = 0u;
				}
				return packed;
			}

			glm::vec3 const center{ (nodeBox.min + nodeBox.max) * 0.5f };
			glm::vec3 const oc{ (obj.min + obj.max) * 0.5f };
			unsigned oct{ 0u };
			if(oc.x >= center.x) oct |= 1u;
			if(oc.y >= center.y) oct |= 2u;
			if(oc.z >= center.z) oct |= 4u;

			AABB childBox{};
			childBox.min = {
				(oct & 1u) ? center.x : nodeBox.min.x,
				(oct & 2u) ? center.y : nodeBox.min.y,
				(oct & 4u) ? center.z : nodeBox.min.z
			};
			childBox.max = {
				(oct & 1u) ? nodeBox.max.x : center.x,
				(oct & 2u) ? nodeBox.max.y : center.y,
				(oct & 4u) ? nodeBox.max.z : center.z
			};

			if(!childBox.contains(obj)) {
				if(depth > 0u) {
					unsigned const pshift{ (depth - 1u) * 4u };
					unsigned pnib{ (packed >> pshift) & 0xFu };
					pnib &= ~0x8u;
					packed = (packed & ~(0xFu << pshift)) | (pnib << pshift);
				}
				else {
					packed = 0u;
				}
				return packed;
			}
			unsigned const shift{ depth * 4u };
			unsigned const nib{ (1u << 3) | (oct & 7u) };
			packed = (packed & ~(0xFu << shift)) | (nib << shift);
			nodeBox = childBox;
			++depth;
		}
	}
}

//std::vector<EntityID> Octree::retrieveVisible(Frustum const &f) {
//	std::vector<EntityID> out;
//	if(!root) return out;
//
//	struct Item {
//		OctNode *n; AABB box;
//	};
//	Item st[256]; int top = -1;
//	st[++top] = { root.get(), world };
//
//	while(top >= 0) {
//		Item it = st[top--];
//
//		// --- frustum vs AABB (center-radius test per plane) ---
//		AABB const &b = it.box;
//		glm::vec3 c = (b.min + b.max) * 0.5f;    // center
//		glm::vec3 e = (b.max - b.min) * 0.5f;    // half-extents
//
//		bool outside = false;
//		int pi = 0;
//		while(pi < 6) {
//			glm::vec3 const &n = f.p[pi].n;      // plane normal (unit)
//			float const d = f.p[pi].d;            // plane offset
//			float r = e.x * std::abs(n.x) + e.y * std::abs(n.y) + e.z * std::abs(n.z);
//			float s = glm::dot(n, c) + d;        // signed distance of center
//			if(s + r < 0.0f) {                   // entirely outside this plane
//				outside = true;
//				break;
//			}
//			++pi;
//		}
//		if(outside) continue;
//
//		// --- emit objects in this node ---
//		ObjNode *o = it.n->head.get();
//		while(o) {
//			out.push_back(o->id);
//			o = o->next.get();
//		}
//
//		// --- push children (compute child AABB inline) ---
//		for(unsigned i = 0; i < 8; ++i) {
//			if(it.n->child[i]) {
//				AABB cb;
//				glm::vec3 mid = (it.box.min + it.box.max) * 0.5f;
//				cb.min = {
//					(i & 1u) ? mid.x : it.box.min.x,
//					(i & 2u) ? mid.y : it.box.min.y,
//					(i & 4u) ? mid.z : it.box.min.z
//				};
//				cb.max = {
//					(i & 1u) ? it.box.max.x : mid.x,
//					(i & 2u) ? it.box.max.y : mid.y,
//					(i & 4u) ? it.box.max.z : mid.z
//				};
//				st[++top] = { it.n->child[i].get(), cb };
//			}
//		}
//	}
//
//	return out;
//}
