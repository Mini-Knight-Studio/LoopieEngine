#include "Octree.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Render/Colors.h"
#include "Loopie/Core/Log.h"

#include <algorithm>

namespace Loopie
{
    LooseOctree::LooseOctree(const AABB& worldBounds)
    {
        m_root = std::make_unique<LooseOctreeNode>();
        m_root->bounds = worldBounds;
        m_root->looseBounds = Expand(worldBounds);
        m_root->depth = 0;

        m_lookup.reserve(500);
    }

    void LooseOctree::Clear()
    {
        AABB b = m_root->bounds;

        m_root = std::make_unique<LooseOctreeNode>();
        m_root->bounds = b;
        m_root->looseBounds = Expand(b);

        m_lookup.clear();
    }

    void LooseOctree::Insert(const std::shared_ptr<Entity>& entity)
    {
        Insert(m_root.get(), entity.get());
    }

    void LooseOctree::Insert(LooseOctreeNode* node, Entity* entity)
    {
        Entity* key = entity;

        if (!key || !(key->GetComponentCount() > 1))
            return;

        AABB box = GetAABB(entity);

        if (node->isLeaf)
        {
            node->entities.push_back(entity);
            m_lookup[key] = node;

            if (node->entities.size() > LO_MAX_ENTITIES && node->depth < LO_MAX_DEPTH)
            {
                Subdivide(node);
                Reinsert(node);
            }
            return;
        }

        for (auto& child : node->children)
        {
            if (child && child->looseBounds.Contains(box))
            {
                Insert(child.get(), entity);
                return;
            }
        }

        node->entities.push_back(entity);
        m_lookup[key] = node;
    }

    void LooseOctree::Remove(const std::shared_ptr<Entity>& entity)
    {
        Entity* key = entity.get();
        if (!key) 
            return;

        auto lookupIt = m_lookup.find(key);
        if (lookupIt == m_lookup.end())
            return;

        LooseOctreeNode* node = lookupIt->second;
        auto& entities = node->entities;

        auto it = std::find(entities.begin(), entities.end(), key);
        if (it != entities.end())
        {
            *it = entities.back();
            entities.pop_back();
        }

        m_lookup.erase(lookupIt);
    }

    void LooseOctree::Update(const std::shared_ptr<Entity>& entity)
    {

        if (!entity || !(entity->GetComponentCount() > 1))
            return;

        Entity* key = entity.get();

        auto it = m_lookup.find(key);
        if (it != m_lookup.end())
        {
            Remove(entity);
        }

        Insert(entity);
    }

    void LooseOctree::Query(const AABB& range, std::vector<Entity*>& result)
    {
        Query(m_root.get(), range, result);
    }

    void LooseOctree::Query(LooseOctreeNode* node,
        const AABB& range,
        std::vector<Entity*>& result)
    {
        if (!node->looseBounds.Intersects(range))
            return;

        for (auto& e : node->entities)
            if (GetAABB(e).Intersects(range))
                result.push_back(e);

        if (!node->isLeaf)
            for (auto& c : node->children)
                if (c) 
                    Query(c.get(), range, result);
    }

    void LooseOctree::Subdivide(LooseOctreeNode* node)
    {
        auto children = ComputeChildren(node->bounds);

        for (int i = 0; i < 8; i++)
        {
            node->children[i] = std::make_unique<LooseOctreeNode>();
            node->children[i]->bounds = children[i];
            node->children[i]->looseBounds = Expand(children[i]);
            node->children[i]->depth = node->depth + 1;
            node->children[i]->parent = node;
        }

        node->isLeaf = false;
    }

    void LooseOctree::Reinsert(LooseOctreeNode* node)
    {
        auto old = std::move(node->entities);
        node->entities.clear();

        for (auto& e : old)
            Insert(node, e);
    }

    void LooseOctree::CollectVisibleEntitiesFrustum(const Frustum& frustum, std::unordered_set<Entity*>& out)
    {
        FrustumQuery(m_root.get(), frustum, out);
    }

    void LooseOctree::FrustumQuery(LooseOctreeNode* node, const Frustum& frustum, std::unordered_set<Entity*>& out)
    {
        if (!node || !frustum.Intersects(node->looseBounds))
            return;

        for (auto& e : node->entities)
            if (frustum.Intersects(GetAABB(e)))
                out.insert(e);

        if (!node->isLeaf)
            for (auto& c : node->children)
                if (c) 
                    FrustumQuery(c.get(), frustum, out);
    }

    void LooseOctree::CollectIntersectingObjectsWithRay(const vec3& origin, const vec3& dir, std::unordered_set<Entity*>& out)
    {
        RayQuery(m_root.get(), origin, dir, out);
    }

    void LooseOctree::RayQuery(LooseOctreeNode* node, const vec3& origin, const vec3& dir, std::unordered_set<Entity*>& out)
    {
        if (!node)
            return;

        vec3 hit;
        if (!node->looseBounds.IntersectsRay(origin, dir, hit))
            return;

        for (auto& e : node->entities)
        {
            vec3 h;
            if (GetAABB(e).IntersectsRay(origin, dir, h))
                out.insert(e);
        }

        if (!node->isLeaf)
            for (auto& c : node->children)
                if (c) RayQuery(c.get(), origin, dir, out);
    }

    AABB LooseOctree::GetAABB(const Entity* e)
    {
        if (auto mesh = e->GetComponent<MeshRenderer>())
            return mesh->GetWorldAABB();

        vec3 p = e->GetTransform()->GetPosition();
        return AABB(p - vec3(0.5f), p + vec3(0.5f));
    }

    std::array<AABB, 8> LooseOctree::ComputeChildren(const AABB& p)
    {
        std::array<AABB, 8> out;

        vec3 c = p.GetCenter();
        vec3 min = p.MinPoint;
        vec3 max = p.MaxPoint;

        for (int i = 0; i < 8; i++)
        {
            vec3 cmin(
                (i & 1) ? c.x : min.x,
                (i & 2) ? c.y : min.y,
                (i & 4) ? c.z : min.z
            );

            vec3 cmax(
                (i & 1) ? max.x : c.x,
                (i & 2) ? max.y : c.y,
                (i & 4) ? max.z : c.z
            );

            out[i] = AABB(cmin, cmax);
        }

        return out;
    }

    AABB LooseOctree::Expand(const AABB& box)
    {
        vec3 c = box.GetCenter();
        vec3 half = (box.MaxPoint - box.MinPoint) * 0.5f * LO_LooseFactor;
        return AABB(c - half, c + half);
    }

    void LooseOctree::DebugDraw(const vec4& color)
    {
        if (m_shouldDraw)
            DebugDrawNode(m_root.get(), color);
    }

    void LooseOctree::DebugDrawNode(LooseOctreeNode* node, const vec4& color)
    {
        if (!node) return;

        Gizmo::DrawCube(node->bounds, Color::MAGENTA);

        for (auto& c : node->children)
            if (c) DebugDrawNode(c.get(),color);
    }
}