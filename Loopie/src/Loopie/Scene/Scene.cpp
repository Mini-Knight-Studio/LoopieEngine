#include "Scene.h"
#include "Loopie/Files/Json.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Animator.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Helpers/LoopieHelpers.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Components/BoxCollider.h"

#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/AudioListener.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Components/Button.h"

#include <unordered_set>


namespace Loopie {
	Scene::Scene(const std::string& filePath)
	{
		m_filePath = filePath;

		m_rootEntity = std::make_shared<Entity>("scene");
		m_rootEntity->AddComponent<Transform>();

		m_octree = std::make_unique<Octree>(DEFAULT_WORLD_BOUNDS);

		Application::GetInstance().m_notifier.AddObserver(this);
	}

	Scene::~Scene()
	{
		Application::GetInstance().m_notifier.RemoveObserver(this);
		m_entities.clear();
	}

	void Scene::SaveScene(const std::string filePath)
	{
		JsonData saveData;
		JsonNode entitiesObj = saveData.CreateArrayField("entities");
		
		for (const auto& [id, entity] : GetAllEntities())
		{
			JsonData entityObj = JsonData();
			
			entityObj.CreateField<std::string>("uuid", id.Get());
			entityObj.CreateField<std::string>("name", entity->GetName());
			entityObj.CreateField<bool>("active", entity->GetIsActiveInHierarchy());


			if (std::shared_ptr<Entity> parentEntity = entity->GetParent().lock())
				entityObj.CreateField<std::string>("parent_uuid", parentEntity->GetUUID().Get());

			// Creates an array of components
			JsonNode componentsObj = entityObj.CreateArrayField("components");
			
			for (auto const& component : entity->GetComponents())
			{
				JsonData componentObj = JsonData();
				component->Serialize(componentObj.Node());
				componentObj.CreateField<std::string>("uuid", component->GetUUID().Get());
				componentsObj.AddArrayElement(componentObj.GetRoot());
			}

			saveData.AddArrayElement("entities", entityObj.GetRoot());
		}

		saveData.ToFile(filePath);
		Log::Info("Scene saved.");
	}

	// *** Octree rebuild TEMP *** - PSS 13/12/25
	// The create entities functions are doing an m_octree->Rebuild();
	// This is a temporal call to the function. The reason is because
	// the after creating the entity, if the entity had no mesh originally
	// it will create an AABB based on its transform. 
	// This makes it so that the Octree will never be fully accurate because
	// the last entity may not be accurate. 
	// A rebuild / update of the entity should be called AFTER the entity gets its mesh.
	std::shared_ptr<Entity> Scene::CreateEntity(const std::string& name,
												std::shared_ptr<Entity> parentEntity)
	{
		std::shared_ptr<Entity> realParent = parentEntity ? parentEntity : m_rootEntity;
		std::string uniqueName = GetUniqueName(realParent, name);
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(uniqueName);

		realParent->AddChild(entity);

		entity->AddComponent<Transform>();

		m_entities[entity->GetUUID()] = entity;
		m_octree->Insert(entity);

		m_octree->Rebuild();
		return entity;
	}

	std::shared_ptr<Entity> Scene::CreateEntity(const UUID& uuid, const std::string& name,
												 std::shared_ptr<Entity> parentEntity)
	{

		std::shared_ptr<Entity> realParent = parentEntity ? parentEntity : m_rootEntity;

		std::string uniqueName = GetUniqueName(realParent, name);
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(uniqueName);
		entity->SetUUID(uuid);

		realParent->AddChild(entity);

		entity->AddComponent<Transform>();

		m_entities[entity->GetUUID()] = entity;
		m_octree->Insert(entity);

		m_octree->Rebuild();
		return entity;
	}

	std::shared_ptr<Entity> Scene::CreateEntity(const vec3& position, const quaternion& rotation, const vec3& scale,
												std::shared_ptr<Entity> parentEntity, const std::string& name)
	{
		std::shared_ptr<Entity> realParent = parentEntity ? parentEntity : m_rootEntity;
		std::string uniqueName = GetUniqueName(realParent, name);
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(uniqueName);

		realParent->AddChild(entity);

		entity->AddComponent<Transform>(position, rotation, scale);
		m_entities[entity->GetUUID()] = entity;
		m_octree->Insert(entity);

		m_octree->Rebuild();
		return entity;
	}

	std::shared_ptr<Entity> Scene::CreateEntity(Transform* transform,
										std::shared_ptr<Entity> parentEntity,
										const std::string& name)
	{
		std::shared_ptr<Entity> realParent = parentEntity ? parentEntity : m_rootEntity;
		std::string uniqueName = GetUniqueName(realParent, name);
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(uniqueName);
		realParent->AddChild(entity);

		if (!transform)
		{
			entity->AddComponent<Transform>();
		}
		else
		{
			entity->AddComponent<Transform>(*transform);
		}
		m_entities[entity->GetUUID()] = entity;
		m_octree->Insert(entity);

		m_octree->Rebuild();
		return entity;
	}

	void Scene::RemoveEntity(UUID uuid)
	{
		auto it = m_entities.find(uuid);
		if (it == m_entities.end())
			return;

		RemoveEntityRecursive(it->second);
		m_octree->Rebuild();
	}

	void Scene::RemoveEntity(std::shared_ptr<Entity> entity)
	{
		if (!entity) 
			return;

		RemoveEntityRecursive(entity);
		m_octree->Rebuild();
	}

	void Scene::RemoveEntityDeferred(UUID uuid)
	{
		if (m_entities.find(uuid) == m_entities.end())
			return;

		m_entitiesPendingDestroy.insert(uuid);
	}

	void Scene::FlushRemovedEntities()
	{
		if (m_entitiesPendingDestroy.empty())
			return;

		for (const UUID& uuid : m_entitiesPendingDestroy)
		{
			auto it = m_entities.find(uuid);
			if (it != m_entities.end())
			{
				RemoveEntityRecursive(it->second);
			}
		}

		m_entitiesPendingDestroy.clear();
		m_octree->Rebuild();
	}

	std::shared_ptr<Entity> Scene::CloneEntity(const std::shared_ptr<Entity> source, std::shared_ptr<Entity> newParent, bool cloneChildren)
	{
		if (!source)
			return nullptr;

		if (!newParent)
			newParent = source->GetParent().lock();

		// Create new entity
		std::shared_ptr<Entity> clone = CreateEntity(
			source->GetName(),
			newParent
		);

		clone->SetIsActive(source->GetIsActiveInHierarchy());

		// ---- Clone components ----
		for (Component* component : source->GetComponents())
		{
			JsonData componentData;
			component->Serialize(componentData.Node());

			// Transform already exists
			if (componentData.Child("transform").IsValid())
			{
				clone->GetTransform()->Deserialize(componentData.Child("transform"));
				continue;
			}
			if (componentData.Child("recttransform").IsValid())
			{
				clone->ReplaceTransform<RectTransform>();
				clone->GetTransform()->Deserialize(componentData.Child("recttransform"));
			}
			// Camera
			if (componentData.Child("camera").IsValid())
			{
				auto cam = clone->AddComponent<Camera>();
				cam->Deserialize(componentData.Child("camera"));
			}
			// MeshRenderer
			else if (componentData.Child("meshrenderer").IsValid())
			{
				auto mr = clone->AddComponent<MeshRenderer>();
				mr->Deserialize(componentData.Child("meshrenderer"));
			}
			// ScriptClass
			else if (componentData.Child("script").IsValid())
			{
				std::string classID = componentData.Child("script").GetValue<std::string>("class_id", "").Result;
				ScriptClass* scriptClass = clone->AddComponent<ScriptClass>(classID);
				scriptClass->Deserialize(componentData.Child("script"));
			}
			/// Animator
			else if (componentData.Child("animator").IsValid())
			{
				auto animator = clone->AddComponent<Animator>();
				animator->Deserialize(componentData.Child("animator"));
			}
			 // Canvas
			else if (componentData.Child("canvas").IsValid())
			{
				auto canvas = clone->AddComponent<Canvas>();
				canvas->Deserialize(componentData.Child("canvas"));
			}
			/// Image
			else if (componentData.Child("image").IsValid())
			{
				auto image = clone->AddComponent<Image>();
				image->Deserialize(componentData.Child("image"));
			}
			/// BoxCollider
			else if (componentData.Child("boxcollider").IsValid())
			{
				auto bc = clone->AddComponent<BoxCollider>();
				bc->Deserialize(componentData.Child("boxcollider"));
			}
			//AudioSource
			else if (componentData.Child("audioSource").IsValid())
			{
				auto audioSource = clone->AddComponent<AudioSource>();
				audioSource->Deserialize(componentData.Child("audioSource"));
			}
			//AudioListener
			else if (componentData.Child("audioListener").IsValid())
			{
				auto audioListener = clone->AddComponent<AudioListener>();
				audioListener->Deserialize(componentData.Child("audioListener"));
			}
			// Text
			else if (componentData.Child("text").IsValid())
			{
				auto text = clone->AddComponent<Text>();
				text->Deserialize(componentData.Child("text"));
			}
			// Button
			else if (componentData.Child("button").IsValid())
			{
				auto button = clone->AddComponent<Button>();
				button->Deserialize(componentData.Child("button"));
			}
		}

		// ---- Clone children ----
		if (cloneChildren)
		{
			for (const auto& child : source->GetChildren())
			{
				CloneEntity(child, clone, true);
			}
		}

		return clone;
	}

	void Scene::SetFilePath(std::string filePath)
	{
		m_filePath = filePath;
	}

	std::string Scene::GetFilePath() const
	{
		return m_filePath;
	}

	std::shared_ptr<Entity> Scene::GetRootEntity() const
	{
		return m_rootEntity;
	}

	std::shared_ptr<Entity> Scene::GetEntity(UUID uuid) const
	{
		auto it = m_entities.find(uuid);
		return (it != m_entities.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Entity> Scene::GetEntity(const std::string& name) const
	{
		for (const auto& [uuid, entity] : m_entities)
		{
			if (entity->GetName() == name)
			{
				return entity;
			}
		}
		return nullptr;
	}

	Octree& Scene::GetOctree() const
	{
		return *m_octree;
	}

	const std::unordered_map<UUID, std::shared_ptr<Entity>>& Scene::GetAllEntities() const
	{
		return m_entities;
	}

	std::vector<std::shared_ptr<Entity>> Scene::GetAllEntitiesHierarchical(std::shared_ptr<Entity> parentEntity) const
	{
		std::vector<std::shared_ptr<Entity>> entities;

		if (!parentEntity)
		{
			parentEntity = m_rootEntity;
		}

		CollectEntitiesRecursive(parentEntity, entities);
		return entities;
	}

	std::vector<std::shared_ptr<Entity>> Scene::GetAllSiblings(std::shared_ptr<Entity> parentEntity) const
	{
		std::vector<std::shared_ptr<Entity>> siblingEntities;

		if (!parentEntity)
		{
			parentEntity = m_rootEntity;
		}

		for (const auto& child : parentEntity->GetChildren())
		{
			siblingEntities.push_back(child);
		}

		return siblingEntities;
	}

	bool Scene::ReadAndLoadSceneFile(std::string filePath, bool safeSceneAsLastLoaded)
	{
		m_entities.clear();
		m_octree->Clear();

		m_octree = std::make_unique<Octree>(DEFAULT_WORLD_BOUNDS);

		m_rootEntity = std::make_shared<Entity>("scene");
		m_rootEntity->AddComponent<Transform>();
		
		JsonData saveData = Json::ReadFromFile(filePath);

		if (saveData.IsEmpty())
		{
			Log::Error("Failed to load scene file or scene does not exist, opening Default...");
			return false;
		}

		JsonNode rootNode = saveData.Child("entities");

		if (!rootNode.IsValid() || !rootNode.IsArray())
		{
			Log::Error("No entities array in scene file.");
			return true;
		}

		// First iteration: Create all entities
		for (unsigned int i = 0; i < rootNode.Size(); ++i)
		{
			JsonResult<json> entityJson = rootNode.GetArrayElement<json>(i);
			JsonNode entityNode = JsonNode(&entityJson.Result);

			if (!entityNode.IsValid() || !entityNode.Contains("uuid") || !entityNode.Contains("name"))
				continue;

			UUID uuid = UUID(entityNode.GetValue<std::string>("uuid").Result);
			std::string name = entityNode.GetValue<std::string>("name").Result;
			bool active = entityNode.GetValue<bool>("active", false).Result;

			std::shared_ptr<Entity> entity = CreateEntity(uuid, name);
			entity->SetName(name);
			entity->SetIsActive(active);		
		}

		// Second iteration: Link relationships and components
		for (size_t i = 0; i < rootNode.Size(); ++i)
		{
			JsonResult<json> entityJson = rootNode.GetArrayElement<json>(uint32_t(i));
			JsonNode entityNode = JsonNode(&entityJson.Result);

			if (!entityNode.IsValid() || !entityNode.Contains("uuid"))
				continue;

			UUID uuid = UUID(entityNode.GetValue<std::string>("uuid").Result);
			std::shared_ptr<Entity> entity = m_entities[uuid];

			// Set parent if it exists
			if (entityNode.Contains("parent_uuid"))
			{
				UUID parentUUID = UUID(entityNode.GetValue<std::string>("parent_uuid").Result);
				if (m_entities.find(parentUUID) != m_entities.end())
				{
					entity->SetParent(m_entities[parentUUID]);
				}
			}

			JsonNode componentsObj = entityNode.Child("components");
			if (componentsObj.IsValid() && componentsObj.IsArray())
			{
				for (size_t j = 0; j < componentsObj.Size(); ++j)
				{
					JsonResult<json> componentJson = componentsObj.GetArrayElement<json>(uint32_t(j));
					JsonNode componentNode = JsonNode(&componentJson.Result);

					JsonResult<std::string> componentUUIDResult = componentNode.GetValue<std::string>("uuid");
					UUID componentUUID = UUID();
					if(componentUUIDResult.Found)
					{
						componentUUID = UUID(componentUUIDResult.Result);
					}

					// *** Component Checking *** - PSS 08/12/25
					// This checks manually which component type it is.
					// This lacks scalability. 
					// Might be worth looking into if components expand too much.
					if (componentNode.Contains("transform"))
					{
						JsonNode node = componentNode.Child("transform");
						entity->GetTransform()->Deserialize(node);
						entity->GetTransform()->SetUUID(componentUUID.Get());
					}
					else if (componentNode.Contains("recttransform"))
					{
						JsonNode node = componentNode.Child("recttransform");
						entity->ReplaceTransform<RectTransform>();
						entity->GetTransform()->Deserialize(node);
						entity->GetTransform()->SetUUID(componentUUID.Get());
					}
					else if (componentNode.Contains("camera"))
					{
						JsonNode node = componentNode.Child("camera");
						auto camera = entity->AddComponent<Camera>();
						if (camera)
						{
							camera->Deserialize(node);
							camera->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("meshrenderer"))
					{
						JsonNode node = componentNode.Child("meshrenderer");
						auto meshRenderer = entity->AddComponent<MeshRenderer>();
						if (meshRenderer)
						{
							meshRenderer->Deserialize(node);
							meshRenderer->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("script"))
					{
						JsonNode node = componentNode.Child("script");
						auto scriptClass = entity->AddComponent<ScriptClass>("");
						if (scriptClass)
						{
							scriptClass->Deserialize(node);
							scriptClass->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("animator"))
					{
						JsonNode node = componentNode.Child("animator");
						auto animatorClass = entity->AddComponent<Animator>();
						if (animatorClass)
						{
							animatorClass->Deserialize(node);
							animatorClass->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("canvas"))
					{
						JsonNode node = componentNode.Child("canvas");
						auto canvas = entity->AddComponent<Canvas>();
						if (canvas)
						{
							canvas->Deserialize(node);
							canvas->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("image"))
					{
						JsonNode node = componentNode.Child("image");
						auto image = entity->AddComponent<Image>();
						if (image)
						{
							image->Deserialize(node);
							image->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("boxcollider"))
					{
						JsonNode node = componentNode.Child("boxcollider");
						auto boxCollider = entity->AddComponent<BoxCollider>();
						if (boxCollider)
						{
							boxCollider->Deserialize(node);
							boxCollider->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("audiosource"))
					{
						JsonNode node = componentNode.Child("audiosource");
						auto audioSource = entity->AddComponent<AudioSource>();
						if (audioSource)
						{
							audioSource->Deserialize(node);
							audioSource->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("audiolistener"))
					{
						JsonNode node = componentNode.Child("audiolistener");
						auto audioListener = entity->AddComponent<AudioListener>();
						if (audioListener)
						{
							audioListener->Deserialize(node);
							audioListener->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("text"))
					{
						JsonNode node = componentNode.Child("text");
						auto text = entity->AddComponent<Text>();
						if (text)
						{
							text->Deserialize(node);
							text->SetUUID(componentUUID.Get());
						}
					}
					else if (componentNode.Contains("button"))
					{
						JsonNode node = componentNode.Child("button");
						auto button = entity->AddComponent<Button>();
						if (button)
						{
							button->Deserialize(node);
							button->SetUUID(componentUUID.Get());
						}
					}
				}
			}
		}

		for (auto& [uuid, entity] : m_entities)
		{
			if (!entity)
				continue;

			auto& components = entity->GetComponents();
			for (auto& component : components)
			{
				if (component)
					component->OnSceneDeserialized();
			}
		}


		Log::Info("Scene loaded successfully");

		if (safeSceneAsLastLoaded) {
			m_filePath = filePath;
			std::filesystem::path config = Application::GetInstance().m_activeProject.GetConfigPath();
			if (!config.empty())
			{
				JsonData configData = Json::ReadFromFile(config.string());
				JsonResult<std::string> result = configData.Child("last_scene").GetValue<std::string>();
				if (!result.Found) {
					configData.CreateField<std::string>("last_scene", "");
				}
				std::filesystem::path relativePath = std::filesystem::relative(filePath, Application::GetInstance().m_activeProject.GetProjectPath().parent_path());
				configData.SetValue<std::string>("last_scene", relativePath.string());
				configData.ToFile(config.string());
			}
		}

		m_octree->Rebuild();

		return true;
	}

	void Scene::OnNotify(const EngineNotification& id)
	{
		if(id == EngineNotification::OnRuntimeStart)
		{
			for(const auto& [uuid, entity] : m_entities)
			{
				std::vector<ScriptClass*> scripts =entity->GetComponents<ScriptClass>();
				for (size_t i = 0; i < scripts.size(); i++)
				{
					scripts[i]->SetUp();
				}
			}
		}
		else if (id == EngineNotification::OnRuntimeStop)
		{
			for (const auto& [uuid, entity] : m_entities)
			{
				std::vector<ScriptClass*> scripts = entity->GetComponents<ScriptClass>();
				for (size_t i = 0; i < scripts.size(); i++)
				{
					scripts[i]->DestroyInstance();
				}
			}
		}
	}

	std::string Scene::GetUniqueName(std::shared_ptr<Entity> parentEntity, const std::string& desiredName)
	{
		if (!parentEntity)
			return desiredName;

		std::vector<std::string> existingNames;
		for (const auto& sibling : GetAllSiblings(parentEntity))
		{
			existingNames.emplace_back(sibling->GetName());
		}

		return Helper::MakeUniqueName(desiredName, existingNames);
	}

	void Scene::CollectEntitiesRecursive(std::shared_ptr<Entity> entity,
		std::vector<std::shared_ptr<Entity>>& outEntities) const
	{
		if (!entity)
			return;

		outEntities.push_back(entity);

		for (const auto& child : entity->GetChildren())
		{
			CollectEntitiesRecursive(child, outEntities);
		}
	}

	void Scene::RemoveEntityRecursive(std::shared_ptr<Entity> entity)
	{
		if (!entity) 
			return;

		auto children = entity->GetChildren(); // Making a copy instead of referencing just in case

		for (const auto& child : children)
		{
			RemoveEntityRecursive(child);
		}

		if (std::shared_ptr<Entity> parent = entity->GetParent().lock())
		{
			parent->RemoveChild(entity->GetUUID());
		}

		m_octree->Remove(entity);
		m_entities.erase(entity->GetUUID());
	}
}
