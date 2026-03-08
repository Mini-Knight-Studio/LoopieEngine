#pragma once

#include "Loopie/Components/BoxCollider.h"
#include "Loopie/Math/Ray.h"
#include <vector>

namespace Loopie {

	constexpr unsigned int MAX_LAYERS = 16;

	struct CollisionLayer
	{
		std::string name;
		uint32_t bit;
	};

	struct RaycastHit
	{
		BoxCollider* collider = nullptr;
		vec3 point = vec3(0.0f);
		float distance = std::numeric_limits<float>::max();
	};

	class CollisionProcessor
	{

	public:

		static void Initialize();

		static void Register(BoxCollider* collider);
		static void Unregister(BoxCollider* collider);

		static bool Raycast(const Ray& ray, RaycastHit& hit); // Now it doesnt use the octree

		static void Process(); // Now it doesnt use the octree

		/// Layers

		static const CollisionLayer& GetLayer(unsigned int  index);
		static const CollisionLayer& GetLayer(const std::string& name);
		static const int GetLayerIndex(const std::string& name);
		static void SetLayerName(unsigned int  index, const std::string& name);

		static const std::array<CollisionLayer, MAX_LAYERS>& GetLayers() { return s_layers; }

		static void ClearMatrix();
		static bool GetLayerCollision(unsigned int a, unsigned int b);
		static void SetLayerCollision(unsigned int a, unsigned int b, bool value);

		static void SaveLayers();
	private:


	private:
		static std::vector<BoxCollider*> s_colliders;
		static std::array<CollisionLayer, MAX_LAYERS> s_layers;

		static bool s_collisionMatrix[MAX_LAYERS][MAX_LAYERS];
	};
}