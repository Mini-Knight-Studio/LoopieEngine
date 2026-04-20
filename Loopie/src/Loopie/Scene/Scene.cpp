#include "Scene.h"
#include "Loopie/Files/Json.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/ParticleComponent.h"
#include "Loopie/Components/Animator.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/CanvasScaler.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/SpriteAnimator.h"
#include "Loopie/Helpers/LoopieHelpers.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Components/BoxCollider.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Audio/AudioManager.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Resources/Types/SceneAsset.h"

#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/AudioListener.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Components/Button.h"
#include "Loopie/Components/Light.h"

#include <unordered_set>


namespace Loopie {
	Scene::Scene()
	{

		m_entities.reserve(2000);

		m_rootEntity = std::make_shared<Entity>("scene");
		m_rootEntity->AddComponent<Transform>();

		m_octree = std::make_unique<LooseOctree>(DEFAULT_WORLD_BOUNDS);

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
			entityObj.CreateField<bool>("static", entity->GetIsStaticInHierarchy());

			if (std::shared_ptr<Entity> parentEntity = entity->GetParent().lock())
				entityObj.CreateField<std::string>("parent_uuid", parentEntity->GetUUID().Get());

			// Creates an array of components
			JsonNode componentsObj = entityObj.CreateArrayField("components");
			
			for (auto const& component : entity->GetComponents())
			{
				JsonData componentObj = JsonData();
				component->Serialize(componentObj.Node());
				componentObj.CreateField<std::string>("uuid", component->GetUUID().Get());
				componentObj.CreateField<bool>("is_active", component->GetLocalIsActive());
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

		OnStaticGeometryChanged();
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

		//OnStaticGeometryChanged(); // Skipping so loading scenes is faster, doing it after all entities are created
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

		OnStaticGeometryChanged();
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

		OnStaticGeometryChanged();
		return entity;
	}

	void Scene::RemoveEntity(UUID uuid)
	{
		auto it = m_entities.find(uuid);
		if (it == m_entities.end())
			return;

		RemoveEntityRecursive(it->second);
		OnStaticGeometryChanged();
	}

	void Scene::RemoveEntity(std::shared_ptr<Entity> entity)
	{
		if (!entity) 
			return;

		RemoveEntityRecursive(entity);
		OnStaticGeometryChanged();
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

		/// DO HERE COMPONENTS THAT CAN BE CLONED WITHOUT DEPENDENCIES FIRST (ex: Transform can be cloned before MeshRenderer because it doesn't require any data from the mesh to be cloned)
		for (Component* component : source->GetComponents())
		{
			if (!component)
				continue;

			size_t compType = component->GetTypeID();

			if (compType == Transform::GetTypeIDStatic())
			{
				clone->GetTransform()->Clone(source, *component);
				continue;
			}

			if (compType == RectTransform::GetTypeIDStatic())
			{
				clone->ReplaceTransform<RectTransform>();
				clone->GetTransform()->Clone(source, *component);
				continue;
			}
			// Camera
			if (compType == Camera::GetTypeIDStatic())
			{
				auto cam = clone->AddComponent<Camera>();
				cam->Clone(source, *component);
				cam->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// MeshRenderer
			else if (compType == MeshRenderer::GetTypeIDStatic())
			{
				auto mr = clone->AddComponent<MeshRenderer>();
				mr->Clone(source, *component);
				mr->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// ParticleComponent
			else if (compType == ParticleComponent::GetTypeIDStatic())
			{
				auto pc = clone->AddComponent<ParticleComponent>();
				pc->Clone(source, *component);
				pc->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// ScriptClass
			else if (compType == ScriptClass::GetTypeIDStatic())
			{
				const ScriptClass* scriptComp = static_cast<const ScriptClass*>(component);
				ScriptClass* scriptClass = clone->AddComponent<ScriptClass>(scriptComp->GetClassName());
				scriptClass->Clone(source, *component);
				scriptClass->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			 // Canvas
			else if (compType == Canvas::GetTypeIDStatic())
			{
				auto canvas = clone->AddComponent<Canvas>();
				canvas->Clone(source, *component);
				canvas->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			/// Image
			else if (compType == Image::GetTypeIDStatic())
			{
				auto image = clone->AddComponent<Image>();
				image->Clone(source, *component);
				image->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			/// SpriteAnimator
			else if (compType == SpriteAnimator::GetTypeIDStatic())
			{
				auto spriteAnimator = clone->AddComponent<SpriteAnimator>();
				spriteAnimator->Clone(source, *component);
				spriteAnimator->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			/// BoxCollider
			else if (compType == BoxCollider::GetTypeIDStatic())
			{
				auto bc = clone->AddComponent<BoxCollider>();
				bc->Clone(source, *component);
				bc->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			//AudioSource
			else if (compType == AudioSource::GetTypeIDStatic())
			{
				auto audioSource = clone->AddComponent<AudioSource>();
				audioSource->Clone(source, *component);
				audioSource->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			//AudioListener
			else if (compType == AudioListener::GetTypeIDStatic())
			{
				auto audioListener = clone->AddComponent<AudioListener>();
				audioListener->Clone(source, *component);
				audioListener->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// Text
			else if (compType == Text::GetTypeIDStatic())
			{
				auto text = clone->AddComponent<Text>();
				text->Clone(source, *component);
				text->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// Button
			else if (compType == Button::GetTypeIDStatic())
			{
				auto button = clone->AddComponent<Button>();
				button->Clone(source, *component);
				button->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// Light
			else if (compType == Light::GetTypeIDStatic())
			{
				auto light = clone->AddComponent<Light>();
				light->Clone(source, *component);
				light->SetIsActive(component->GetLocalIsActive());
				continue;
			}
			// CanvasScaler
			else if (compType == CanvasScaler::GetTypeIDStatic())
			{
				auto scaler = clone->AddComponent<CanvasScaler>();
				scaler->Clone(source, *component);
				scaler->SetIsActive(component->GetLocalIsActive());
				continue;
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

		///// DO HERE COMPONENTS THAT REQUIRE OTHER COMPONENTS TO BE CLONED FIRST (ex: Animator needs MeshRenderer to be cloned first to properly clone the renderer data)

		for (Component* component : source->GetComponents())
		{
			if (!component)
				continue;

			size_t compType = component->GetTypeID();

			/// Animator
			if (compType == Animator::GetTypeIDStatic())
			{
				auto animator = clone->AddComponent<Animator>();
				animator->Clone(source, *component);
				animator->SetIsActive(component->GetLocalIsActive());
				continue;
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

	void Scene::OnStaticGeometryChanged()
	{
		UpdateEntitySpanningBounds();
		Renderer::SetShadowsDirty();
	}

	LooseOctree& Scene::GetOctree() const
	{
		return *m_octree;
	}

	const AABB& Scene::GetEntitySpanningBounds() const
	{
		return m_entitySpanningBounds;
	}

	void Scene::UpdateEntitySpanningBounds()
	{
		m_entitySpanningBounds = m_octree->ComputeSceneAABB();
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

	bool Scene::ReadAndLoadSceneFile(const UUID& uuid)
	{
		Metadata* metadata = AssetRegistry::GetMetadata(uuid);
		if (!metadata)
			return false;

		std::shared_ptr<SceneAsset> sceneAsset = ResourceManager::GetSceneAsset(*metadata);
		if (!sceneAsset)
			return false;
		if (!sceneAsset->Load())
			return false;

		
		return ReadAndLoadSceneFile((Application::GetInstance().m_activeProject.GetChachePath() / sceneAsset->GetSceneFilePath()).string(), false);
	}

	bool Scene::ReadAndLoadSceneFile(std::string filePath, bool safeSceneAsLastLoaded)
	{
		m_loadRequest = false;
		m_entities.clear();
		m_octree->Clear();

		m_octree = std::make_unique<LooseOctree>(DEFAULT_WORLD_BOUNDS);

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
			bool isStatic = entityNode.GetValue<bool>("static", true).Result;

			std::shared_ptr<Entity> entity = CreateEntity(uuid, name, nullptr);
			entity->SetName(name);
			entity->SetIsActive(active);		
			entity->SetIsStatic(isStatic);
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
					JsonResult<bool> componentActiveResult = componentNode.GetValue<bool>("is_active", true);
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
							camera->SetIsActive(componentActiveResult.Result);
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
							meshRenderer->SetIsActive(componentActiveResult.Result);
						}
					}
					else if (componentNode.Contains("particlecomponent"))
					{
						JsonNode node = componentNode.Child("particlecomponent");
						auto particleComponent = entity->AddComponent<ParticleComponent>();
						if (particleComponent)
						{
							particleComponent->Deserialize(node);
							particleComponent->SetUUID(componentUUID.Get());
							particleComponent->SetIsActive(componentActiveResult.Result);
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
							scriptClass->SetIsActive(componentActiveResult.Result);
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
							animatorClass->SetIsActive(componentActiveResult.Result);
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
							canvas->SetIsActive(componentActiveResult.Result);
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
							image->SetIsActive(componentActiveResult.Result);
						}
					}
					else if (componentNode.Contains("sprite_animator"))
					{
						JsonNode node = componentNode.Child("sprite_animator");
						auto spriteAnimator = entity->AddComponent<SpriteAnimator>();
						if (spriteAnimator)
						{
							spriteAnimator->Deserialize(node);
							spriteAnimator->SetUUID(componentUUID.Get());
							spriteAnimator->SetIsActive(componentActiveResult.Result);
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
							boxCollider->SetIsActive(componentActiveResult.Result);
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
							audioSource->SetIsActive(componentActiveResult.Result);
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
							audioListener->SetIsActive(componentActiveResult.Result);
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
							text->SetIsActive(componentActiveResult.Result);
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
							button->SetIsActive(componentActiveResult.Result);
						}
					}
					else if (componentNode.Contains("light"))
					{
						JsonNode node = componentNode.Child("light");
						auto light = entity->AddComponent<Light>();
						if (light)
						{
							light->Deserialize(node);
							light->SetUUID(componentUUID.Get());
							light->SetIsActive(componentActiveResult.Result);
						}
					}
					else if (componentNode.Contains("canvas_scaler"))
					{
						JsonNode node = componentNode.Child("canvas_scaler");
						auto scaler = entity->AddComponent<CanvasScaler>();
						if (scaler)
						{
							scaler->Deserialize(node);
							scaler->SetUUID(componentUUID.Get());
							scaler->SetIsActive(componentActiveResult.Result);
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

		if(ScriptingManager::IsRunning())
		{
			Application::GetInstance().m_notifier.Notify(EngineNotification::OnRuntimeStart);
		}
		

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

		for (const auto& [uuid, entity] : GetAllEntities()) {
			if (entity->GetTransform()->HasChangedThisFrame()) {
				m_octree->Update(entity);
				entity->GetTransform()->CleanChangesFlag();
			}
		}

		OnStaticGeometryChanged();

		return true;
	}

	bool Scene::RequestLoad(const UUID& uuid)
	{
		Metadata* metadata = AssetRegistry::GetMetadata(uuid);
		if (!metadata)
			return false;

		std::shared_ptr<SceneAsset> sceneAsset = ResourceManager::GetSceneAsset(*metadata);
		if (!sceneAsset)
			return false;
		if (!sceneAsset->Load())
			return false;

		m_loadRequest = true;
		m_requestedSceneToLoad = uuid;

		return true;
	}

	void Scene::OnNotify(const EngineNotification& id)
	{
		if(id == EngineNotification::OnRuntimeStart)
		{
			AudioManager::StartSceneAudio(this);

			for(const auto& [uuid, entity] : m_entities)
			{
				std::vector<ScriptClass*> scripts =entity->GetComponents<ScriptClass>();
				for (size_t i = 0; i < scripts.size(); i++)
				{
					scripts[i]->SetUp();
				}
			}

			for (const auto& [uuid, entity] : m_entities)
			{
				std::vector<ScriptClass*> scripts = entity->GetComponents<ScriptClass>();
				for (size_t i = 0; i < scripts.size(); i++)
				{
					scripts[i]->InvokeOnCreate();
				}
			}

			OnStaticGeometryChanged();
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

		const auto& siblings = parentEntity->GetChildren();

		auto exists = [&](const std::string& name) -> bool
			{
				for (const auto& child : siblings)
				{
					if (child->GetName() == name)
						return true;
				}
				return false;
			};

		if (!exists(desiredName))
			return desiredName;

		for (int i = 1;; ++i)
		{
			std::string candidate = desiredName + "_" + std::to_string(i);

			if (!exists(candidate))
				return candidate;
		}
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


		if (ScriptingManager::IsRunning()) {
			auto destroy = [&](std::vector<ScriptClass*>& components) -> void {
				for (size_t i = 0; i < components.size(); i++)
					components[i]->InvokeOnDestroy();
			};

			destroy(entity->GetComponents<ScriptClass>());
		}
		

		m_octree->Remove(entity);
		m_entities.erase(entity->GetUUID());
		OnStaticGeometryChanged();
	}
}
