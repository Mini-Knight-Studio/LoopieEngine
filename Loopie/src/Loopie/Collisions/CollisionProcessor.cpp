#include "CollisionProcessor.h"

#include "Loopie/Math/AABB.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Scene/Entity.h"

#include <algorithm>

namespace Loopie {

	std::vector<BoxCollider*> CollisionProcessor::s_colliders;


	void CollisionProcessor::Register(BoxCollider* collider) {
		s_colliders.push_back(collider);
	}

	void CollisionProcessor::Unregister(BoxCollider* collider) {
		s_colliders.erase(std::remove(s_colliders.begin(), s_colliders.end(), collider), s_colliders.end());
	}

    bool CollisionProcessor::Raycast(const Ray& ray, RaycastHit& hit)
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
}