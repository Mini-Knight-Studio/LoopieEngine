#pragma once

#include "Loopie/Components/BoxCollider.h"
#include <vector>

namespace Loopie {
	class CollisionProcessor
	{
	public:

		static void Register(BoxCollider* collider);
		static void Unregister(BoxCollider* collider);

		static void Process();
	private:
		static std::vector<BoxCollider*> s_colliders;
	};
}