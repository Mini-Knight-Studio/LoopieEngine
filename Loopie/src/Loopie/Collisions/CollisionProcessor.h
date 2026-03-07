#pragma once

#include "Loopie/Components/BoxCollider.h"
#include "Loopie/Math/Ray.h"
#include <vector>

namespace Loopie {

	struct RaycastHit
	{
		BoxCollider* collider = nullptr;
		vec3 point = vec3(0.0f);
		float distance = std::numeric_limits<float>::max();
	};

	class CollisionProcessor
	{

	public:


		static void Register(BoxCollider* collider);
		static void Unregister(BoxCollider* collider);

		static bool Raycast(const Ray& ray, RaycastHit& hit); // Now it doesnt use the octree

		static void Process(); // Now it doesnt use the octree
	private:
		static std::vector<BoxCollider*> s_colliders;
	};
}