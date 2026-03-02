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

	void CollisionProcessor::Process() {
        if (s_colliders.empty())
            return;

        std::sort(s_colliders.begin(), s_colliders.end(),
            [](BoxCollider* a, BoxCollider* b)
            {
                return a->GetWorldAABB().MinPoint.x < b->GetWorldAABB().MinPoint.x;
            });

        const int count = (int)s_colliders.size();

        for (auto* colider : s_colliders)
        {
            colider->m_colliding = false;
            colider->m_collided = false;
            colider->m_stopColliding = false;
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