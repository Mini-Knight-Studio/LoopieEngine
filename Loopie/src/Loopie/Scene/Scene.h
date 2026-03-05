#pragma once

#include "Loopie/Core/UUID.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Math/Octree.h"

#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"

#include <string>
#include <unordered_map>
	
namespace Loopie {
	class Scene : public IObserver<EngineNotification>
	{
	public:
		Scene(const std::string& filePath);
		~Scene();

		void SaveScene(const std::string filePath = nullptr);

		std::shared_ptr<Entity> CreateEntity(const std::string& name = "Entity",
											 std::shared_ptr<Entity> parentEntity = nullptr);
		std::shared_ptr<Entity> CreateEntity(const UUID& uuid, 
											const std::string& name = "Entity",
											std::shared_ptr<Entity> parentEntity = nullptr);
		std::shared_ptr<Entity> CreateEntity(const vec3& position = { 0, 0, 0 }, 
											const quaternion& rotation = { 1, 0, 0, 0 }, 
											const vec3& scale = { 1, 1, 1 },
											std::shared_ptr<Entity> parentEntity = nullptr,
											const std::string& name = "Entity");
		std::shared_ptr<Entity> CreateEntity(Transform* transform = nullptr,
											std::shared_ptr<Entity> parentEntity = nullptr,
											const std::string& name = "Entity");
		
		void RemoveEntity(UUID uuid);
		void RemoveEntity(std::shared_ptr<Entity> entity);

		void RemoveEntityDeferred(UUID uuid);
		void FlushRemovedEntities();

		std::shared_ptr<Entity> CloneEntity(const std::shared_ptr<Entity> source, std::shared_ptr<Entity> newParent = nullptr, bool cloneChildren = true);

		void SetFilePath(std::string filePath);

		std::string GetFilePath() const;
		std::shared_ptr<Entity> GetRootEntity() const;
		std::shared_ptr<Entity> GetEntity(UUID uuid) const;
		std::shared_ptr<Entity> GetEntity(const std::string& name) const;
		Octree& GetOctree() const;
		// Returns the unordered_map of the UUID and Entity ptrs
		// Usage: for (const auto& [uuid, entity] : scene.GetAllEntities()) {entity->Update();}
		const std::unordered_map<UUID, std::shared_ptr<Entity>>& GetAllEntities() const;
		std::vector<std::shared_ptr<Entity>> GetAllEntitiesHierarchical(std::shared_ptr<Entity> parentEntity = nullptr) const;
		std::vector<std::shared_ptr<Entity>> GetAllSiblings(std::shared_ptr<Entity> parentEntity = nullptr) const;
		bool ReadAndLoadSceneFile(std::string filePath, bool safeSceneAsLastLoaded = true);

	public:

	private:
		void OnNotify(const EngineNotification& id) override;
		std::string GetUniqueName(std::shared_ptr<Entity> parentEntity, const std::string& desiredName);
		void CollectEntitiesRecursive(std::shared_ptr<Entity> entity,
									  std::vector<std::shared_ptr<Entity>>& outEntities) const;
		void RemoveEntityRecursive(std::shared_ptr<Entity> parent);

	private:
		std::unique_ptr<Octree> m_octree;
		std::unordered_map<UUID, std::shared_ptr<Entity>> m_entities; // Fast lookup
		std::unordered_set<UUID> m_entitiesPendingDestroy;
		std::shared_ptr<Entity> m_rootEntity; // Hierarchy based
		std::string m_filePath;
		const AABB DEFAULT_WORLD_BOUNDS = AABB(vec3(-500, -450, -500), vec3(500, 550, 500));
	};	
}