#include "Octree.h"

#include "Loopie/Math/Frustum.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Colors.h"
#include "Loopie/Render/Gizmo.h"


namespace Loopie
{
	// *** World Bounds as a constructor *** - PSS 14/12/2025
	// Scene holds DEFAULT_WORLD_BOUNDS which holds the bounds for the world
	// Keep in mind - it should be best to avoid the exact center, otherwise
	// if we place things at height 0, it will collide with all quadrants.
	Octree::Octree(const AABB& rootBounds)
	{
		m_rootNode = std::make_unique<OctreeNode>(rootBounds);
	}

	// *** Steps of Insert *** - PSS 14/12/2025
	// 1. From the root node it goes downwards and inspects the node's children's AABB
	//    and does a test with the Entity's AABB.
	//    If there's a collision, it does this recursively with that child.
	//    If multiple children collide with that entity's AABB, the parent keeps the entity and 
	//    we stop recursion.
	// 2. If you run out of Bucket Space and you are a leaf, subdivide node
	//	  into 8 child nodes and add them in the parent node's list.
	// 3. Redistribute all GameObjects to proper childs based on their
	//    position in space.
	// 4. If it reaches maximum depth, stop subdividing and add the object in
	//    the list of entities of the node even we are over max entities (8) capacity.
	void Octree::Insert(std::shared_ptr<Entity> entity)
	{
		AABB entityAABB = GetEntityAABB(entity);
		InsertRecursively(m_rootNode.get(), entity, entityAABB, 0);
	}

	void Octree::Remove(std::shared_ptr<Entity> entity)
	{
		AABB entityAABB = GetEntityAABB(entity);
		RemoveRecursively(m_rootNode.get(), entity, entityAABB);
	}

	// This can be optimized if it's too slow
	void Octree::Update(std::shared_ptr<Entity> entity)
	{
		Remove(entity);
		Insert(entity);
	}

	void Octree::Clear()
	{
		if (m_rootNode)
		{
			m_rootNode.reset();
		}
	}

	// *** How Rebuild works *** - PSS 14/12/2025
	// This allows us to rebalance the tree if it becomes too unbalanced, for example
	// if internal nodes hold too many children.
	// Right now it has to be called manually, but we could get the statistics
	// of the Octree and rebuild automatically given a certain statistic,
	// for example after X frames, or if an internal node holds too many entities.
	void Octree::Rebuild()
	{
		AABB rootBounds = m_rootNode->m_aabb;
		//std::vector<std::shared_ptr<Entity>> allEntities;
		std::unordered_set<std::shared_ptr<Entity>> allEntities;
		CollectAllEntitiesFromNode(m_rootNode.get(), allEntities);

		Clear();

		m_rootNode = std::make_unique<OctreeNode>(rootBounds);
		m_rootNode->m_isLeaf = true;

		for (auto& entity : allEntities)
		{
			Insert(entity);
		}
	}

	// *** Debug Draw *** - PSS 14/12/2025
	// This debug draws the whole Octree. We might consider doing optimizations, 
	// like frustrum, and expand it to debug from a certain Octree downwards
	void Octree::DebugDraw(const vec4& color)
	{
		if (m_shouldDraw)
		{
			DebugDrawRecursively(m_rootNode.get(), color, 0);
		}
	}

	void Octree::DebugPrintOctreeStatistics()
	{
		OctreeStatistics statistics = GetStatistics();
		Log::Info("==========================");
		Log::Info("Printing Octree Statistics");
		Log::Info("==========================");
		Log::Info("Total Nodes = {0}", statistics.totalNodes);
		Log::Info("Leaf Nodes = {0}", statistics.leafNodes);
		Log::Info("Internal Nodes = {0}", statistics.internalNodes);
		Log::Info("Total Entities = {0}", statistics.totalEntities);
		// The line below could be implemented with a few changes.
		//Log::Info("Total Visible Entities = {0}", statistics.totalEntities); 
		Log::Info("Max Depth = {0}", statistics.maxDepth);
		Log::Info("Min Entities Per Node = {0}", statistics.minEntitiesPerNode);
		Log::Info("Max Entities Per Node = {0}", statistics.maxEntitiesPerNode);
		Log::Info("Average Entities Per Node = {0}", statistics.averageEntitiesPerNode);
		Log::Info("Empty Leaves = {0}", statistics.emptyNodes);
		Log::Info("Overfilled Leaves = {0}", statistics.overfilledNodes);
	}
	
	void Octree::DebugPrintOctreeHierarchy()
	{
		Log::Info("==========================");
		Log::Info("Printing Octree Hierarchy");
		Log::Info("==========================");
		
		DebugPrintOctreeHierarchyRecursively(m_rootNode.get(), 0);
	}

	OctreeStatistics Octree::GetStatistics() const
	{
		OctreeStatistics stats;
		GatherStatisticsRecursively(m_rootNode.get(), stats, 0);

		if (stats.leafNodes > 0)
		{
			stats.averageEntitiesPerNode = static_cast<float>(stats.totalEntities) / stats.leafNodes;
		}

		return stats;
	}

	// *** Variable rayHit does nothing at the moment *** - PSS 14/12/25
	// I haven't programmed the rayhit to return any meaningful value at the moment or to do anything with it
	// I will have to check on it at some point
	void Octree::CollectIntersectingObjectsWithRay(vec3 rayOrigin, vec3 rayDirection,
												   std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		vec3 rayHit;
		CollectIntersectingObjectsWithRayRecursively(m_rootNode.get(), rayOrigin, rayDirection, rayHit, entities);
	}

	void Octree::CollectIntersectingObjectsWithAABB(const AABB& queryBox,
													std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		CollectIntersectingObjectsWithAABBRecursively(m_rootNode.get(), queryBox, entities);
	}

	void Octree::CollectIntersectingObjectsWithSphere(const vec3& center, const float& radius,
													  std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		CollectIntersectingObjectsWithSphereRecursively(m_rootNode.get(), center, radius, entities);
	}

	void Octree::CollectVisibleEntitiesFrustum(const Frustum& frustum, 
											   std::unordered_set<std::shared_ptr<Entity>>& visibleEntities)
	{
		CollectVisibleEntitiesFrustumRecursively(m_rootNode.get(), frustum, visibleEntities);
	}

	void Octree::CollectAllEntities(std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		CollectAllEntitiesFromNode(m_rootNode.get(), entities);
	}

	void Octree::SetShouldDraw(bool value)
	{
		m_shouldDraw = value;
	}

	void Octree::ToggleShouldDraw()
	{
		m_shouldDraw = !m_shouldDraw;
	}

	bool Octree::GetShouldDraw() const
	{
		return m_shouldDraw;
	}

	AABB Octree::GetEntityAABB(const std::shared_ptr<Entity>& entity) const
	{
		auto meshRenderer = entity->GetComponent<MeshRenderer>();
		if (meshRenderer)
		{
			return meshRenderer->GetWorldAABB();
		}
		/*else
		{
			vec3 entityPosition = entity->GetTransform()->GetPosition();
			AABB aabb(entityPosition);
			
			return aabb;
		}*/

		Transform* transform = entity->GetTransform();
		if(!transform)
		{
			vec3 entityPosition = entity->GetTransform()->GetPosition();
			AABB aabb(entityPosition);
			
			return aabb;
		}

		const vec3 localMin = transform->GetLocalBoundsMin();
		const vec3 localMax = transform->GetLocalBoundsMax();

		if (localMin == vec3(0.0f) && localMax == vec3(0.0f))
		{
			vec3 entityPosition = transform->GetPosition();
			AABB aabb(entityPosition);
			
			return aabb;
		}

		const matrix4& m = transform->GetLocalToWorldMatrix();
		const vec3 corners[8] = 
		{
			vec3(localMin.x, localMin.y, localMin.z),
			vec3(localMax.x, localMin.y, localMin.z),
			vec3(localMin.x, localMax.y, localMin.z),
			vec3(localMax.x, localMax.y, localMin.z),
			
			vec3(localMin.x, localMin.y, localMax.z),
			vec3(localMax.x, localMin.y, localMax.z),
			vec3(localMin.x, localMax.y, localMax.z),
			vec3(localMax.x, localMax.y, localMax.z)
		};

		vec3 minW = vec3(m * vec4(corners[0], 1.0f));
		vec3 maxW = minW;

		for (int i = 1; i < 8; ++i)
		{
			vec3 worldCorner = vec3(m * vec4(corners[i], 1.0f));
			minW = glm::min(minW, worldCorner);
			maxW = glm::max(maxW, worldCorner);
		}

		return AABB(minW, maxW);
	}
	
	void Octree::InsertRecursively(OctreeNode* node, std::shared_ptr<Entity> entity, const AABB& entityAABB, int depth)
	{
		if (!node)
		{
			return;
		}

		// If the node is a leaf, add entity and check for entities and depth.
		// If max capacity has reached and hasn't reached max depth, 
		// it will subdivide the node and redistribute all entities.
		if (node->m_isLeaf)
		{
			node->m_entities.insert(entity);

			if (node->m_entities.size() > MAX_ENTITIES_PER_NODE && depth < MAXIMUM_DEPTH)
			{
				Subdivide(node);
				RedistributeEntities(node, depth);
			}
			return;
		}

		// If it entity intersects with multiple children, put entity on their parent node
		int totalNodesIntersecting = 0;
		int nodeNumberFound = -1;

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i] && node->m_children[i]->m_aabb.Intersects(entityAABB))
			{
				totalNodesIntersecting++;
				if (totalNodesIntersecting > 1)
				{
					node->m_entities.insert(entity);
					return;
				}
				nodeNumberFound = i;
			}
		}

		// If found a node intersecting with entity and hasn't been inserted yet, 
		// try to insert below that node
		if (totalNodesIntersecting == 1)
		{
			InsertRecursively(node->m_children[nodeNumberFound].get(), entity, entityAABB, depth + 1);
		}
		else
		{
			// No children intersect - store at this node (edge case)
			node->m_entities.insert(entity);
		}
	}

	void Octree::RemoveRecursively(OctreeNode* node, std::shared_ptr<Entity> entity, const AABB& entityAABB)
	{
		if (!node)
		{
			return;
		}
		
		// Early exit - if the entities' AABB doesn't intersect with this node's AABB, skip it and its children.
		if (!entityAABB.Intersects(node->m_aabb))
		{
			return;
		}

		auto it = node->m_entities.find(entity);
		bool foundEntity = (it != node->m_entities.end());

		if (foundEntity)
		{
			node->m_entities.erase(entity);
			return;
		}

		int totalNodesIntersecting = 0;
		int nodeNumberFound = -1;

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i] && node->m_children[i]->m_aabb.Intersects(entityAABB))
			{
				totalNodesIntersecting++;
				nodeNumberFound = i;
			}
		}

		if (totalNodesIntersecting == 1)
		{
			RemoveRecursively(node->m_children[nodeNumberFound].get(), entity, entityAABB);
		}
	}

	void Octree::Subdivide(OctreeNode* node)
	{
		// Subdivide node into 8 different nodes
		std::array<AABB, MAX_ENTITIES_PER_NODE> childAABBs = ComputeChildAABBs(node->m_aabb);

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			node->m_children[i] = std::make_unique<OctreeNode>(childAABBs[i]);
			node->m_children[i]->m_parent = node;
			node->m_children[i]->m_isLeaf = true;
		}

		node->m_isLeaf = false;
	}

	void Octree::RedistributeEntities(OctreeNode* node, int depth)
	{
		// Redistribute entities into those 8 different nodes
		if (node->m_isLeaf)
		{
			return;
		}

		std::unordered_set<std::shared_ptr<Entity>> entitiesToRedistribute = std::move(node->m_entities);

		for (auto& entity : entitiesToRedistribute)
		{
			AABB entityAABB = GetEntityAABB(entity);


			// If it entity intersects with multiple children, put entity on their parent node
			int totalNodesIntersecting = 0;
			int nodeNumberFound = -1;

			for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
			{
				if (node->m_children[i] && node->m_children[i]->m_aabb.Intersects(entityAABB))
				{
					totalNodesIntersecting++;
					if (totalNodesIntersecting > 1)
					{
						break;
					}
					nodeNumberFound = i;
				}
			}

			if (totalNodesIntersecting > 1)
			{
				node->m_entities.insert(entity);
			}
			else if (totalNodesIntersecting == 1)
			{
				node->m_children[nodeNumberFound]->m_entities.insert(entity);
			}
			else
			{
				// Intersects with no children (edge case) -> Parent keeps entity
				node->m_entities.insert(entity);
			}
		}

		// After redistribution, check each child if it needs further subdivision
		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i] && node->m_children[i]->m_entities.size() > MAX_ENTITIES_PER_NODE
				&& depth + 1 < MAXIMUM_DEPTH)
			{
				Subdivide(node->m_children[i].get());
				RedistributeEntities(node->m_children[i].get(), depth + 1);
			}
		}
	}

	std::array<AABB, MAX_ENTITIES_PER_NODE> Octree::ComputeChildAABBs(const AABB& parentAABB) const
	{
		std::array<AABB, MAX_ENTITIES_PER_NODE> children;
		vec3 center = parentAABB.GetCenter();
		vec3 min = parentAABB.MinPoint;
		vec3 max = parentAABB.MaxPoint;

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			// This generates all octants by using bit-wise operations
			vec3 childMin, childMax;
			childMin.x = (i & 1) ? center.x : min.x;
			childMin.y = (i & 2) ? center.y : min.y;
			childMin.z = (i & 4) ? center.z : min.z;
			childMax.x = (i & 1) ? max.x : center.x;
			childMax.y = (i & 2) ? max.y : center.y;
			childMax.z = (i & 4) ? max.z : center.z;

			AABB aabb(childMin, childMax);
			children[i] = aabb;
		}

		return children;
	}

	void Octree::DebugDrawRecursively(OctreeNode* node, const vec4& color, int depth)
	{
		if (!node)
		{
			return;
		}

		// If is Leaf and depth is greater than 3, then color is YELLOW
		if (node->m_isLeaf && depth > 3)
		{
			Gizmo::DrawCube(node->m_aabb.MinPoint, node->m_aabb.MaxPoint, Color::YELLOW);
		}
		// The rest's is WHATEVER color contained within var color (normally GREEN)
		else
		{
			Gizmo::DrawCube(node->m_aabb.MinPoint, node->m_aabb.MaxPoint, color);
		}

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{			
			if (node->m_children[i])
			{
				DebugDrawRecursively(node->m_children[i].get(), color, depth + 1);
			}
		}
	}

	void Octree::DebugPrintOctreeHierarchyRecursively(OctreeNode* node, int depth) const
	{
		if (!node)
		{
			return;
		}
		Log::Info("Node depth = {0}. Entities in node = {1}", depth, node->m_entities.size());

		if (node->m_isLeaf)
		{
			return;
		}

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			DebugPrintOctreeHierarchyRecursively(node->m_children[i].get(), depth + 1);
		}
	}

	void Octree::GatherStatisticsRecursively(OctreeNode* node, OctreeStatistics& stats, int depth) const
	{
		if (!node)
		{
			return;
		}

		stats.totalNodes++;
		stats.maxDepth = std::max(stats.maxDepth, depth);

		int entityCount = static_cast<int>(node->m_entities.size());
		stats.totalEntities += entityCount;

		stats.minEntitiesPerNode = std::min(stats.minEntitiesPerNode, entityCount);
		stats.maxEntitiesPerNode = std::max(stats.maxEntitiesPerNode, entityCount);

		if (entityCount == 0)
		{
			stats.emptyNodes++;
		}

		if (entityCount > MAX_ENTITIES_PER_NODE && depth >= MAXIMUM_DEPTH)
		{
			stats.overfilledNodes++;
		}

		if (node->m_isLeaf)
		{
			stats.leafNodes++;
		}
		else
		{
			stats.internalNodes++;

			for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
			{
				if (node->m_children[i])
				{
					GatherStatisticsRecursively(node->m_children[i].get(), stats, depth + 1);
				}
			}
		}
	}

	void Octree::CollectAllEntitiesFromNode(OctreeNode* node, std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		if (!node)
		{
			return;
		}

		for (const auto& entity : node->m_entities)
		{
			entities.insert(entity);
		}

		if (!node->m_isLeaf)
		{
			for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
			{
				if (node->m_children[i])
				{
					CollectAllEntitiesFromNode(node->m_children[i].get(), entities);
				}
			}
		}
	}

	void Octree::CollectIntersectingObjectsWithRayRecursively(OctreeNode* node, vec3 rayOrigin, vec3 rayDirection, vec3& rayHit,
															  std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		if (!node)
		{
			return;
		}

		// Early exit - if the ray doesn't intersect with this node's AABB, skip it and its children.
		if (!node->m_aabb.IntersectsRay(rayOrigin, rayDirection, rayHit))
		{
			return;
		}

		for (const auto& entity : node->m_entities)
		{
			if (GetEntityAABB(entity).IntersectsRay(rayOrigin, rayDirection, rayHit))
			{
				entities.insert(entity);
			}
		}

		if (node->m_isLeaf)
		{
			return;
		}

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i])
			{
				CollectIntersectingObjectsWithRayRecursively(node->m_children[i].get(), rayOrigin, rayDirection, rayHit, entities);
			}
		}
	}

	void Octree::CollectIntersectingObjectsWithAABBRecursively(OctreeNode* node, const AABB& queryBox,
																std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		if (!node)
		{
			return;
		}

		// Early exit - if the queryBox' AABB doesn't intersect with this node's AABB, skip it and its children.
		if (!queryBox.Intersects(node->m_aabb))
		{
			return;
		}

		for (const auto& entity : node->m_entities)
		{
			if (GetEntityAABB(entity).Intersects(queryBox))
			{
				entities.insert(entity);
			}
		}

		if (node->m_isLeaf)
		{
			return;
		}

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i])
			{
				CollectIntersectingObjectsWithAABBRecursively(node->m_children[i].get(), queryBox, entities);
			}
		}
	}

	void Octree::CollectIntersectingObjectsWithSphereRecursively(OctreeNode* node, const vec3& center, const float& radius,
																 std::unordered_set<std::shared_ptr<Entity>>& entities)
	{
		if (!node)
		{
			return;
		}

		// Early exit - if the sphere doesn't intersect with this node's AABB, skip it and its children.
		if (!node->m_aabb.IntersectsSphere(center, radius))
		{
			return;
		}

		for (const auto& entity : node->m_entities)
		{
			if (GetEntityAABB(entity).IntersectsSphere(center, radius))
			{
				entities.insert(entity);
			}
		}

		if (node->m_isLeaf)
		{
			return;
		}

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i])
			{
				CollectIntersectingObjectsWithSphereRecursively(node->m_children[i].get(), center, radius, entities);
			}
		}
	}

	void Octree::CollectVisibleEntitiesFrustumRecursively(OctreeNode* node, const Frustum& frustum,
														  std::unordered_set<std::shared_ptr<Entity>>& visibleEntities)
	{
		if (!node)
		{
			return;
		}

		// Early exit - if the frustum' AABB doesn't intersect with this node's AABB, skip it and its children.
		if (!frustum.Intersects(node->m_aabb))
		{
			return;
		}

		for (const auto& entity : node->m_entities)
		{
			if (frustum.Intersects(GetEntityAABB(entity)))
			{
				visibleEntities.insert(entity);
			}
		}

		if (node->m_isLeaf)
		{
			return;
		}

		for (int i = 0; i < MAX_ENTITIES_PER_NODE; ++i)
		{
			if (node->m_children[i])
			{
				CollectVisibleEntitiesFrustumRecursively(node->m_children[i].get(), frustum, visibleEntities);
			}
		}
	}
}