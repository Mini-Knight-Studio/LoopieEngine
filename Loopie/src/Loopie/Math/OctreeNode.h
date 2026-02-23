#pragma once
#include "Loopie/Math/AABB.h"

#include <memory>
#include <array>
#include <unordered_set>

namespace Loopie
{
	// *** Separated Constants *** - PSS
	// NUM_CHILDREN is inherent to octrees (always 8) and should never change.
	// MAX_ENTITIES_PER_NODE is the tunable bucket capacity threshold before subdivision.
	constexpr int NUM_CHILDREN = 8;
	constexpr int MAX_ENTITIES_PER_NODE = 8;

	class Entity;

	class OctreeNode
	{
		friend class Octree;
	public:
		// explicit added to prevent accidental conversions
		explicit OctreeNode(const AABB& aabb);
		~OctreeNode() = default;

	private:
		AABB m_aabb;
		std::unordered_set<std::shared_ptr<Entity>> m_entities;

		OctreeNode* m_parent = nullptr;
		std::array<std::unique_ptr<OctreeNode>, NUM_CHILDREN> m_children = {};
		bool m_isLeaf = true;
	};
}