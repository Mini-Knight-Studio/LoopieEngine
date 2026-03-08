#include "CollisionProcessor.h"

#include "Loopie/Math/AABB.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Scene/Entity.h"

#include "Loopie/Project/ProjectConfig.h"

#include <algorithm>

namespace Loopie {

	std::vector<BoxCollider*> CollisionProcessor::s_colliders;
    bool CollisionProcessor::s_collisionMatrix[MAX_LAYERS][MAX_LAYERS];
    std::array<CollisionLayer, MAX_LAYERS> CollisionProcessor::s_layers;

    void CollisionProcessor::Initialize()
    {
        for (size_t i = 0; i < MAX_LAYERS; ++i)
            for (size_t j = 0; j < MAX_LAYERS; ++j)
                s_collisionMatrix[i][j] = true;


        for (unsigned int i = 0; i < MAX_LAYERS; i++)
        {
            s_layers[i].bit = 1u << i;
        }
        s_layers[0].name = "Default";
    }

	void CollisionProcessor::Register(BoxCollider* collider) {
		s_colliders.push_back(collider);
	}

	void CollisionProcessor::Unregister(BoxCollider* collider) {
		s_colliders.erase(std::remove(s_colliders.begin(), s_colliders.end(), collider), s_colliders.end());
	}

    bool CollisionProcessor::Raycast(const Ray& ray, RaycastHit& hit, int layerMask)
    {
        if (s_colliders.empty())
            return false;

        float rayMinX = std::min(ray.StartPoint().x, ray.EndPoint().x);
        float rayMaxX = std::max(ray.StartPoint().x, ray.EndPoint().x);

        bool foundHit = false;

        for (auto* collider : s_colliders)
        {
            if (!collider || !collider->GetIsActive())
                continue;

            int colliderBit = collider->GetLayerBit();
            if ((layerMask & colliderBit) == 0)
                continue;

            const AABB& aabb = collider->GetWorldAABB();

            if (aabb.MinPoint.x > rayMaxX)
                break;

            if (aabb.MaxPoint.x < rayMinX)
                continue;

            vec3 aabbHit;
            if (!aabb.IntersectsRay(ray.StartPoint(), ray.Direction(), aabbHit))
                continue;

            vec3 obbHit;
            if (!collider->GetWorldOBB().IntersectsRay(ray.StartPoint(), ray.Direction(), obbHit))
                continue;

            float dist = glm::length(obbHit - ray.StartPoint());

            if (dist <= glm::length(ray.EndPoint() - ray.StartPoint()) && dist < hit.distance)
            {
                hit.collider = collider;
                hit.point = obbHit;
                hit.distance = dist;
                foundHit = true;
            }
        }

        return foundHit;
    }

	void CollisionProcessor::Process() {
        if (s_colliders.empty())
            return;

        std::sort(s_colliders.begin(), s_colliders.end(),
            [](BoxCollider* a, BoxCollider* b)
            {
                return a->GetWorldAABB().MinPoint.x < b->GetWorldAABB().MinPoint.x;
            });

        const int count = (int)s_colliders.size();

        for (auto* collider : s_colliders)
        {
            collider->m_colliding = false;
            collider->m_collided = false;
            collider->m_stopColliding = false;
            collider->m_collidingWith.clear();
        }

        for (int i = 0; i < count; ++i)
        {
            BoxCollider* collider1 = s_colliders[i];
            if (!collider1->GetIsActive())
                continue;

            const AABB& aAABB = collider1->GetWorldAABB();
            float aMaxX = aAABB.MaxPoint.x;

            for (int j = i + 1; j < count; ++j)
            {
                BoxCollider* collider2 = s_colliders[j];
                if (!collider2->GetIsActive())
                    continue;

                unsigned int layerA = collider1->GetLayerIndex();
                unsigned int layerB = collider2->GetLayerIndex();

                if (!s_collisionMatrix[layerA][layerB])
                    continue;

                const AABB& bAABB = collider2->GetWorldAABB();

                if (bAABB.MinPoint.x > aMaxX)
                    break;

                if (collider1->Intersects(collider2))
                {
                    collider1->m_colliding = true;
                    collider2->m_colliding = true;

                    collider1->m_collidingWith.push_back(collider2);
                    collider2->m_collidingWith.push_back(collider1);

                    if (!collider1->m_wasCollidingLastFrame)
                        collider1->m_collided = true;

                    if (!collider2->m_wasCollidingLastFrame)
                        collider2->m_collided = true;
                }
            }
        }

        for (auto* collider : s_colliders)
        {
            if (collider->m_wasCollidingLastFrame && !collider->m_colliding)
                collider->m_stopColliding = true;

            collider->m_wasCollidingLastFrame = collider->m_colliding;
        }
        
    }
    const CollisionLayer& CollisionProcessor::GetLayer(unsigned int index)
    {
        return s_layers[index];
    }
    const CollisionLayer& CollisionProcessor::GetLayer(const std::string& name)
    {
        for (const auto& layer : s_layers)
        {
            if (layer.name == name)
                return layer;
        }

        return s_layers[0];
    }
    const int CollisionProcessor::GetLayerIndex(const std::string& name)
    {
        int index = 0;
        for (const auto& layer : s_layers)
        {
            if (layer.name == name)
                return index;
            index++;
        }
        return -1;
    }
    void CollisionProcessor::SetLayerName(unsigned int  index, const std::string& name)
    {
        if (index == 0 || index >= MAX_LAYERS)
            return;

        if (name.empty())
            return;

        for (unsigned int i = 0; i < MAX_LAYERS; i++)
        {
            if (i != index && s_layers[i].name == name)
                return;
        }

        s_layers[index].name = name;
    }

    void CollisionProcessor::ClearMatrix()
    {
        for (size_t i = 0; i < MAX_LAYERS; ++i)
            for (size_t j = 0; j < MAX_LAYERS; ++j)
				s_collisionMatrix[i][j] = false;
    }

    bool CollisionProcessor::GetLayerCollision(unsigned int a, unsigned int b)
    {
        if (a >= MAX_LAYERS || b >= MAX_LAYERS)
            return false;

        return s_collisionMatrix[a][b];
    }
    void CollisionProcessor::SetLayerCollision(unsigned int a, unsigned int b, bool value)
    {
        if (a >= MAX_LAYERS || b >= MAX_LAYERS)
            return;

        s_collisionMatrix[a][b] = value;
        s_collisionMatrix[b][a] = value;
    }

    void CollisionProcessor::SaveLayers()
    {
        JsonData data = ProjectConfig::GetData();
        JsonNode layersNode = data.Child("engine_config").Child("collision_layers");
        if(layersNode.IsValid())
        {
            for (unsigned int i = 0; i < MAX_LAYERS; i++)
            {
                layersNode.SetValue(std::to_string(i), s_layers[i].name);
            }
            
		} 

        JsonNode matrixNode = data.Child("engine_config").Child("collision_matrix");

        if (matrixNode.IsValid()) {
            for (size_t i = 0; i < MAX_LAYERS; i++) {
                JsonNode rowNode = matrixNode.Child(std::to_string(i));
                if (!rowNode.IsValid())
                    rowNode = matrixNode.CreateObjectField(std::to_string(i));

                for (size_t j = 0; j < MAX_LAYERS; j++) {
                    rowNode.SetValue(std::to_string(j), s_collisionMatrix[i][j]);
                }
            }
        }

        ProjectConfig::Save(data);
    }
}