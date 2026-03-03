#pragma once
#include "Loopie/Math/OctreeNode.h"

#include <memory>
#include <array>
#include <unordered_map>


namespace Loopie {
	constexpr int MAXIMUM_DEPTH = 5; // Can be modified as necessary

	class Entity;
	struct Frustum;

	struct OctreeStatistics
	{
		int totalNodes = 0;
		int leafNodes = 0;
		int internalNodes = 0;
		int totalEntities = 0;
		//int visibleEntities = 0;
		int maxDepth = 0;
		int minEntitiesPerNode = INT_MAX;
		int maxEntitiesPerNode = 0;
		float averageEntitiesPerNode = 0.0f;
		int emptyNodes = 0;
		int overfilledNodes = 0; // Leaves exceeding MAX_ENTITIES_PER_NODE at max depth
	};

	//template<typename T>
	class Octree
	{
		// *** Possible functions to add *** - PSS 30/11/2025
		// FindNearest (useful for AI, click selection, etc.)
		// Template - Query (return all objects of specific template type given an AABB / sphere)
		// Optimize (if we see that the performance of the Octree is bad)

	public:
		Octree(const AABB& rootBounds);
		~Octree() = default;

		void Insert(const std::shared_ptr<Entity>& entity);
		void Remove(const std::shared_ptr<Entity>& entity);
		void Update(const std::shared_ptr<Entity>& entity);
		void Clear();
		void Rebuild();
		void DebugDraw(const vec4& color);
		void DebugPrintOctreeStatistics();
		void DebugPrintOctreeHierarchy();
		OctreeStatistics GetStatistics() const;
		void CollectIntersectingObjectsWithRay(vec3 rayOrigin, vec3 rayDirection,
			std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectIntersectingObjectsWithAABB(const AABB& queryBox,
			std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectIntersectingObjectsWithSphere(const vec3& center, const float& radius,
			std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectVisibleEntitiesFrustum(const Frustum& frustum,
			std::unordered_set<std::shared_ptr<Entity>>& visibleEntities);

		void CollectAllEntities(std::unordered_set<std::shared_ptr<Entity>>& entities);
		void SetShouldDraw(bool value);
		void ToggleShouldDraw();
		bool GetShouldDraw() const;


	private:
		AABB GetEntityAABB(const std::shared_ptr<Entity>& entity) const;
		void InsertRecursively(OctreeNode* node, const std::shared_ptr<Entity>& entity, const AABB& entityAABB, int depth);
		void RemoveRecursively(OctreeNode* node, const std::shared_ptr<Entity>& entity, const AABB& entityAABB);
		void Subdivide(OctreeNode* node);
		void RedistributeEntities(OctreeNode* node, int depth);
		std::array<AABB, NUM_CHILDREN> ComputeChildAABBs(const AABB& parentAABB) const;
		void DebugDrawRecursively(OctreeNode* node, const vec4& color, int depth);
		void DebugPrintOctreeHierarchyRecursively(OctreeNode* node, int depth) const;
		void GatherStatisticsRecursively(OctreeNode* node, OctreeStatistics& stats, int depth) const;

		void CollectAllEntitiesFromNode(OctreeNode* node, std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectIntersectingObjectsWithRayRecursively(OctreeNode* node, vec3 rayOrigin, vec3 rayDirection, vec3& rayHit,
			std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectIntersectingObjectsWithAABBRecursively(OctreeNode* node, const AABB& queryBox,
			std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectIntersectingObjectsWithSphereRecursively(OctreeNode* node, const vec3& center, const float& radius,
			std::unordered_set<std::shared_ptr<Entity>>& entities);

		void CollectVisibleEntitiesFrustumRecursively(OctreeNode* node, const Frustum& frustum,
			std::unordered_set<std::shared_ptr<Entity>>& visibleEntities);

	private:
		std::unique_ptr<OctreeNode> m_rootNode;
		AABB m_rootBounds; // Stored so Clear() can recreate the root node
		bool m_shouldDraw = false;

		// *** Entity-to-Node Lookup *** - PSS 22/02/2026
		// Maps each entity to the node it's stored in.
		// This allows O(1) removal regardless of whether the entity has moved,
		// fixing the bug where Remove() would fail to find moved entities
		// because it traversed using the entity's current (new) AABB.
		std::unordered_map<Entity*, OctreeNode*> m_entityToNode;
	};
}