#pragma once

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>

#include "Loopie/Math/MathTypes.h"
#include "Loopie/Math/AABB.h"
#include "Loopie/Math/Frustum.h"
#include "Loopie/Scene/Entity.h"

namespace Loopie
{
    constexpr int LO_MAX_DEPTH = 5;
    constexpr int LO_MAX_ENTITIES = 8;
    constexpr float LO_LooseFactor = 1.5f;

    struct LooseOctreeNode
    {
        AABB bounds;
        AABB looseBounds;

        std::vector<Entity*> entities;
        std::array<std::unique_ptr<LooseOctreeNode>, 8> children;

        LooseOctreeNode* parent = nullptr;
        bool isLeaf = true;
        int depth = 0;
    };

    class LooseOctree
    {
    public:
        LooseOctree(const AABB& worldBounds);

        void Insert(const std::shared_ptr<Entity>& entity);
        void Remove(const std::shared_ptr<Entity>& entity);
        void Update(const std::shared_ptr<Entity>& entity);

        void Query(const AABB& range, std::vector<Entity*>& result);

        void CollectVisibleEntitiesFrustum(const Frustum& frustum, std::unordered_set<Entity*>& visibleEntities);

        void CollectIntersectingObjectsWithRay(const vec3& origin, const vec3& dir, std::unordered_set<Entity*>& result);

        void Clear();

        void SetShouldDraw(bool v) { m_shouldDraw = v; }
        void ToggleShouldDraw() { m_shouldDraw = !m_shouldDraw; }
        bool GetShouldDraw() const { return m_shouldDraw; }

        void DebugDraw(const vec4& color);

    private:
        std::unique_ptr<LooseOctreeNode> m_root;
        bool m_shouldDraw = false;

        std::unordered_map<Entity*, LooseOctreeNode*> m_lookup;

    private:
        void Insert(LooseOctreeNode* node, Entity* entity);
        bool Remove(LooseOctreeNode* node, Entity* entity);

        void Query(LooseOctreeNode* node,const AABB& range, std::vector<Entity*>& result);

        void Subdivide(LooseOctreeNode* node);
        void Reinsert(LooseOctreeNode* node);

        void FrustumQuery(LooseOctreeNode* node, const Frustum& frustum, std::unordered_set<Entity*>& out);

        void RayQuery(LooseOctreeNode* node, const vec3& origin, const vec3& dir, std::unordered_set<Entity*>& out);

        std::array<AABB, 8> ComputeChildren(const AABB& parent);
        AABB Expand(const AABB& box);
        AABB GetAABB(const Entity* e);

        void DebugDrawNode(LooseOctreeNode* node, const vec4& color);
    };
}