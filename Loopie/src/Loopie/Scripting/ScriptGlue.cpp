#include "ScriptGlue.h"
#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Components/ScriptClass.h"

#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Animator.h"
#include "Loopie/Components/BoxCollider.h"
#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/AudioListener.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Components/ParticleComponent.h"
#include "Loopie/Components/SpriteAnimator.h"
#include "Loopie/Components/Button.h"

#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"

#include "Loopie/Core/UUID.h"
#include "Loopie/Core/InputEventManager.h"

#include "Loopie/Collisions/CollisionProcessor.h"
#include "Loopie/Audio/AudioManager.h"

#include "Loopie/Render/Gizmo.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Assert.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <algorithm>

namespace Loopie
{
	static std::unordered_map<_MonoType*, std::function<bool(std::shared_ptr<Entity>)>> s_EntityHasComponentFuncs;
	static std::unordered_map<_MonoType*, std::function<UUID(std::shared_ptr<Entity>,int)>> s_EntityGetComponentFuncs;

	namespace Utils {
		std::string MonoStringToString(MonoString* string)
		{
			char* cStr = mono_string_to_utf8(string);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}

		Scene* GetScene()
		{
			Scene* scene = &Application::GetInstance().GetScene();
			if (!scene)
			{
				Log::Error("Scene not found");
				return nullptr;
			}
			return scene;
		}

		std::shared_ptr<Entity> GetEntity(MonoString* entityID)
		{
			Scene* scene = GetScene();
			if (!scene)
				return nullptr;
			UUID uuid(MonoStringToString(entityID));
			if (uuid == UUID::Invalid) {
				Log::Error("Object not Found");
				return nullptr;
			}
			std::shared_ptr<Entity> entity = scene->GetEntity(uuid);
			if (!entity)
				Log::Error("Entity {} not found", uuid.Get());
			return entity;
		}

		std::shared_ptr<Entity> GetEntity(const std::string& name)
		{
			Scene* scene = GetScene();
			if (!scene)
				return nullptr;
			std::shared_ptr<Entity> entity = scene->GetEntity(name);
			if (!entity)
				Log::Error("Entity {} not found", name);
			return entity;
		}

		std::shared_ptr<Entity> GetEntityNoLog(MonoString* entityID)
		{
			Scene* scene = GetScene();
			if (!scene)
				return nullptr;
			UUID uuid(MonoStringToString(entityID));
			if (uuid == UUID::Invalid) {
				return nullptr;
			}
			std::shared_ptr<Entity> entity = scene->GetEntity(uuid);
			return entity;
		}

		std::shared_ptr<Entity> GetEntityNoLog(const std::string& name)
		{
			Scene* scene = GetScene();
			if (!scene)
				return nullptr;
			std::shared_ptr<Entity> entity = scene->GetEntity(name);
			return entity;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		static T* GetComponent(std::shared_ptr<Entity> entity, MonoString* componentID)
		{
			if (!entity)
				return nullptr;
			UUID componentUUID(MonoStringToString(componentID));
			if (componentUUID == UUID::Invalid) {
				Log::Error("Object not Found");
				return nullptr;
			}
			T* component = entity->GetComponent<T>(componentUUID);
			if (!component)
				Log::Error("Component {} not found", componentUUID.Get());
			return component;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		T* GetComponent(std::shared_ptr<Entity> entity)
		{
			if (!entity)
				return nullptr;
			T* component = entity->GetComponent<T>();
			if (!component)
				Log::Error("Component {} not found", T::GetIdentificableName());
			return component;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		T* GetComponent(std::shared_ptr<Entity> entity, int index)
		{
			if (!entity)
				return nullptr;
			T* component = entity->GetComponent<T>(index);
			if (!component)
				Log::Error("Component {} not found", T::GetIdentificableName());
			return component;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		static T* GetComponentNoLog(std::shared_ptr<Entity> entity, MonoString* componentID)
		{
			if (!entity)
				return nullptr;
			UUID componentUUID(MonoStringToString(componentID));
			if (componentUUID == UUID::Invalid) {
				return nullptr;
			}
			T* component = entity->GetComponent<T>(componentUUID);
			return component;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		T* GetComponentNoLog(std::shared_ptr<Entity> entity)
		{
			if (!entity)
				return nullptr;
			T* component = entity->GetComponent<T>();
			return component;
		}
	}

	#define ADD_INTERNAL_CALL(Name) mono_add_internal_call("Loopie.InternalCalls::" #Name, Name)

#pragma region Log
	static void NativeLog(MonoString* parameter, int mode)
	{
		std::string str = Utils::MonoStringToString(parameter);
		Log::ByType(mode,"{0}", str);
	}

	static void NativeLog_Int(int parameter, int mode)
	{
		Log::ByType(mode, "{0}", parameter);
	}

	static void NativeLog_Float(float parameter, int mode)
	{
		Log::ByType(mode, "{0}", parameter);
	}

	static void NativeLog_Vector2(vec2* parameter, int mode)
	{
		Log::ByType(mode, "{0} {1}", parameter->x, parameter->y);
	}

	static void NativeLog_Vector3(vec3* parameter, int mode)
	{
		Log::ByType(mode, "{0} {1} {2}", parameter->x, parameter->y, parameter->z);
	}

	static void NativeLog_Vector4(vec4* parameter, int mode)
	{
		Log::ByType(mode, "{0} {1} {2} {3}", parameter->x, parameter->y, parameter->z, parameter->w);
	}
#pragma endregion

#pragma region Components
	static MonoObject* Entity_GetScriptInstance(MonoString* entityID, MonoString* componentFullName, int index)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if(!entity)
			return nullptr;

		std::vector<ScriptClass*> scriptComponents = entity->GetComponents<ScriptClass>();

		std::string compFullName = Utils::MonoStringToString(componentFullName);
		int indexFind = 0;
		for (ScriptClass* script : scriptComponents)
		{
			if (!script || !script->GetScriptingClass())
				continue;

			if (script->IsSameType(compFullName))
			{
				if(indexFind==index)
					return script->GetInstance();
				indexFind++;
			}
		}

		return nullptr;
	}

	static bool Component_IsActive(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Component* component = Utils::GetComponent<Component>(entity, componentID);
		if (!component)
			return false;
		return component->GetIsActive();
	}

	static void Component_SetActive(MonoString* entityID, MonoString* componentID, MonoBoolean isActive)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Component* component = Utils::GetComponent<Component>(entity, componentID);
		if (!component)
			return;
		component->SetIsActive(isActive!=0);
	}

	static MonoString* Entity_Create(MonoString* entityName, MonoString* parentID)
	{
		Scene* scene = Utils::GetScene();
		if(!scene)
			return ScriptingManager::CreateString("");

		std::shared_ptr<Entity> parent = nullptr;
		if (parentID !=nullptr)
		{
			UUID parentUuid(Utils::MonoStringToString(parentID));
			if (parentUuid == UUID::Invalid) {
				Log::Warn("Invalid UUID: {}", parentUuid.Get());
				return ScriptingManager::CreateString("");
			}
			parent = scene->GetEntity(parentUuid);
			if (!parent) {
				Log::Warn("Parent entity {} not found", parentUuid.Get());
				return ScriptingManager::CreateString("");
			}
		}

		std::string name = Utils::MonoStringToString(entityName);
		std::shared_ptr<Entity> entity = scene->CreateEntity(name, parent);
		if(!entity)
			return ScriptingManager::CreateString("");

		return ScriptingManager::CreateString(entity->GetUUID().Get().c_str());
	}

	static void Entity_Destroy(MonoString* entityID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Utils::GetScene()->RemoveEntityDeferred(entity->GetUUID());
	}

	static MonoString* Entity_Clone(MonoString* entityID, MonoBoolean cloneChilds)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");

		std::shared_ptr<Entity> clone = Utils::GetScene()->CloneEntity(entity, nullptr, (cloneChilds != 0));
		if (!clone)
			return ScriptingManager::CreateString("");

		auto setUp = [&](std::vector<ScriptClass*>& components) -> void {
			for (size_t i = 0; i < components.size(); i++)
				components[i]->SetUp();
		};
		auto create = [&](std::vector<ScriptClass*>& components) -> void {
			for (size_t i = 0; i < components.size(); i++)
				components[i]->InvokeOnCreate();
		};

		setUp(clone->GetComponents<ScriptClass>());
		for (const auto& child : clone->GetChildren())
			setUp(child->GetComponents<ScriptClass>());
		create(clone->GetComponents<ScriptClass>());
		for (const auto& child : clone->GetChildren())
			create(child->GetComponents<ScriptClass>());

		return ScriptingManager::CreateString(clone->GetUUID().Get().c_str());
	}

	static MonoBoolean Entity_AddComponent(MonoString* entityID, MonoString* componentType, MonoString** componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;

		std::string type = Utils::MonoStringToString(componentType);
		std::shared_ptr<ScriptingClass> scriptingClass = ScriptingManager::GetScriptingClass(type);
		if (!scriptingClass) {
			Log::Error("Could not find scripting class {}", type);
			return false;
		}

		ScriptClass* scriptComponent = entity->AddComponent<ScriptClass>(scriptingClass->GetFullName());
		*componentID = ScriptingManager::CreateString(scriptComponent->GetUUID().Get().c_str());
		scriptComponent->SetUp();
		scriptComponent->InvokeOnCreate();
		return true;
	}

	static MonoBoolean Entity_HasComponent(MonoString* entityID, MonoReflectionType* componentType)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;

		MonoType* managedType = mono_reflection_type_get_type(componentType);

		if (s_EntityHasComponentFuncs.find(managedType) == s_EntityHasComponentFuncs.end())
			return false;
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static MonoBoolean Entity_GetComponent(MonoString* entityID, MonoReflectionType* componentType, int index, MonoString** componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		if (s_EntityHasComponentFuncs.find(managedType) == s_EntityHasComponentFuncs.end())
			return false;
		UUID componentUUID = s_EntityGetComponentFuncs.at(managedType)(entity, index);
		if (componentUUID == UUID::Invalid)
			return false;

		*componentID = ScriptingManager::CreateString(componentUUID.Get().c_str());
		return true;
	}

	static MonoString* Entity_FindEntityByName(MonoString* name)
	{
		std::string entityName = Utils::MonoStringToString(name);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityName);
		if (!entity)
			return ScriptingManager::CreateString("");
		return ScriptingManager::CreateString(entity->GetUUID().Get().c_str());
	}

	static MonoString* Entity_FindEntityByID(MonoString* entityID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		return ScriptingManager::CreateString(entity->GetUUID().Get().c_str());
	}

	static void Entity_SetActive(MonoString* entityID, MonoBoolean active) {

		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->SetIsActive(active != 0);
	}

	static MonoBoolean Entity_IsActive(MonoString* entityID) {

		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		return entity->GetIsActive();
	}

	static MonoBoolean Entity_IsActiveInHierarchy(MonoString* entityID) {

		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		return entity->GetIsActiveInHierarchy();
	}

	static MonoString* Entity_GetParent(MonoString* entityID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");

		std::shared_ptr<Entity> parent = entity->GetParent().lock();
		if (!parent)
			return ScriptingManager::CreateString("");

		return ScriptingManager::CreateString(parent->GetUUID().Get().c_str());
	}

	static void Entity_SetParent(MonoString* entityID, MonoString* parentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;

		if (!parentID)
		{
			entity->SetParent(nullptr);
			return;
		}

		std::shared_ptr<Entity> parent = Utils::GetEntity(parentID);
		if (!parent)
			return;

		entity->SetParent(parent);
	}

	static MonoString* Entity_GetName(MonoString* entityID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");

		return ScriptingManager::CreateString(entity->GetName().c_str());
	}

	static void Entity_SetName(MonoString* entityID, MonoString* name)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;

		std::string newName = Utils::MonoStringToString(name);
		entity->SetName(newName);
	}

	static int Entity_GetChildCount(MonoString* entityID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;

		return entity->GetChildCount();
	}

	static MonoString* Entity_GetChild(MonoString* entityID, int index)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");

		std::shared_ptr<Entity> child = entity->GetChild(index);
		if (!child)
			return ScriptingManager::CreateString("");

		return ScriptingManager::CreateString(child->GetUUID().Get().c_str());
	}

	static MonoString* Entity_GetChildByName(MonoString* entityID, MonoString* entityName, MonoBoolean deepSearch)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");

		std::string name = Utils::MonoStringToString(entityName);
		std::shared_ptr<Entity> child = entity->GetChild(name, deepSearch != 0);
		if (!child)
			return ScriptingManager::CreateString("");

		return ScriptingManager::CreateString(child->GetUUID().Get().c_str());
	}

#pragma endregion

#pragma region Transform
	static void Transform_GetPosition(MonoString* entityID, vec3* position)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*position = entity->GetTransform()->GetPosition();
	}

	static void Transform_SetPosition(MonoString* entityID, vec3* position)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->SetPosition(*position);
	}

	static void Transform_GetLocalPosition(MonoString* entityID, vec3* position)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*position = entity->GetTransform()->GetLocalPosition();
	}

	static void Transform_SetLocalPosition(MonoString* entityID, vec3* position)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->SetLocalPosition(*position);
	}

	static void Transform_GetRotation(MonoString* entityID, vec3* rotation)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*rotation = entity->GetTransform()->GetEulerAngles();
	}

	static void Transform_SetRotation(MonoString* entityID, vec3* rotation)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->SetEulerAngles(*rotation);
	}

	static void Transform_GetLocalRotation(MonoString* entityID, vec3* rotation)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*rotation = entity->GetTransform()->GetLocalEulerAngles();
	}

	static void Transform_SetLocalRotation(MonoString* entityID, vec3* rotation)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->SetLocalEulerAngles(*rotation);
	}

	static void Transform_GetLocalScale(MonoString* entityID, vec3* scale)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*scale = entity->GetTransform()->GetLocalScale();
	}

	static void Transform_SetLocalScale(MonoString* entityID, vec3* scale)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->SetLocalScale(*scale);
	}

	static void Transform_Translate(MonoString* entityID, vec3* translation, ObjectSpace space)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->Translate(*translation, (ObjectSpace)space);
	}

	static void Transform_Rotate(MonoString* entityID, vec3* eulerAngles, ObjectSpace space)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->Rotate(*eulerAngles, (ObjectSpace)space);
	}

	static void Transform_LookAt(MonoString* entityID, vec3* target, vec3* worldUp)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		entity->GetTransform()->LookAt(*target, *worldUp);
	}

	static void Transform_Forward(MonoString* entityID, vec3* forward)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*forward = entity->GetTransform()->Forward();
	}

	static void Transform_Back(MonoString* entityID, vec3* back)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*back = entity->GetTransform()->Back();
	}

	static void Transform_Up(MonoString* entityID, vec3* up)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*up = entity->GetTransform()->Up();
	}

	static void Transform_Down(MonoString* entityID, vec3* down)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*down = entity->GetTransform()->Down();
	}

	static void Transform_Left(MonoString* entityID, vec3* left)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*left = entity->GetTransform()->Left();
	}

	static void Transform_Right(MonoString* entityID, vec3* right)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		*right = entity->GetTransform()->Right();
	}
#pragma endregion

#pragma region Animator
	static void Animator_Stop(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			animator->Stop();
	}

	static void Animator_PlayClip(MonoString* entityID, MonoString* componentID, MonoString* clipName, float transitionTime)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			std::string name = Utils::MonoStringToString(clipName);
			animator->Play(name, transitionTime);
		}
	}

	static void Animator_Play(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			animator->Play();
	}

	static void Animator_Pause(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			animator->Pause();
	}

	static void Animator_Resume(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			animator->Resume();
	}

	static int Animator_GetCurrentClipIndex(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return -1;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->GetCurrentClipIndex();
		return -1;
	}

	static MonoString* Animator_GetCurrentClipName(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			const AnimationClip* clip = animator->GetCurrentClip();
			if (clip)
			{
				std::string clipName = clip->Name;
				return ScriptingManager::CreateString(clipName.c_str());
			}
		}
		return ScriptingManager::CreateString("");
	}

	static float Animator_GetCurrentClipDuration(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			return animator->GetCurrentClipDuration();
		}
		return 0.0f;
	}

	static int Animator_GetNextClipIndex(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return -1;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->GetNextClipIndex();
		return -1;
	}

	static MonoString* Animator_GetNextClipName(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			const AnimationClip* clip = animator->GetNextClip();
			if (clip)
			{
				std::string clipName = clip->Name;
				return ScriptingManager::CreateString(clipName.c_str());
			}
		}
		return ScriptingManager::CreateString("");
	}

	static float Animator_GetNextClipDuration(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			return animator->GetNextClipDuration();
		}
		return 0.0f;
	}


	static float Animator_GetClipDurationByIndex(MonoString* entityID, MonoString* componentID, int clipIndex)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			return animator->GetClipDuration(animator->GetClipByIndex(clipIndex));
		}
		return 0.0f;
	}

	static float Animator_GetClipDurationByName(MonoString* entityID, MonoString* componentID, MonoString* clipName)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			return animator->GetClipDuration(animator->GetClipByName(Utils::MonoStringToString(clipName)));
		}
		return 0.0f;
	}

	static float Animator_GetClipIndex(MonoString* entityID, MonoString* componentID, MonoString* clipName)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return -1;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			return animator->GetClipIndex(animator->GetClipByName(Utils::MonoStringToString(clipName)));
		}
		return -1;
	}

	static MonoString* Animator_GetClipName(MonoString* entityID, MonoString* componentID, int clipIndex)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			const AnimationClip* clip = animator->GetClipByIndex(clipIndex);
			if (clip)
			{
				std::string clipName = clip->Name;
				return ScriptingManager::CreateString(clipName.c_str());
			}
		}
		return ScriptingManager::CreateString("");
	}

	static float Animator_GetPlaybackSpeed(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->GetPlaybackSpeed();
		return 0.0f;
	}

	static void Animator_SetPlaybackSpeed(MonoString* entityID, MonoString* componentID, float speed)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			animator->SetPlaybackSpeed(speed);
	}

	static MonoBoolean Animator_IsLooping(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->IsLooping();
		return false;
	}

	static MonoBoolean Animator_IsPlaying(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->IsPlaying();
		return false;
	}

	static MonoBoolean Animator_IsInTransition(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->IsInTransition();
		return false;
	}

	static void Animator_SetLooping(MonoString* entityID, MonoString* componentID, MonoBoolean loop)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			animator->SetLooping(loop != 0);
	}

	static float Animator_GetCurrentTime(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator)
			return animator->GetCurrentTime();
		return 0.0f;
	}

#pragma endregion

#pragma region SpriteAnimator
	static MonoString* SpriteAnimator_GetTextureUUID(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (!animator)
			return ScriptingManager::CreateString("");

		auto texture = animator->GetTexture();
		if (!texture)
			return ScriptingManager::CreateString("");
		return ScriptingManager::CreateString(texture->GetUUID().Get().c_str());
	}

	static void SpriteAnimator_SetTextureUUID(MonoString* entityID, MonoString* componentID, MonoString* textureUUID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (!animator)
			return;

		std::string uuidStr = textureUUID ? Utils::MonoStringToString(textureUUID) : "";
		if (uuidStr.empty())
		{
			animator->SetTexture(std::shared_ptr<Texture>());
			return;
		}

		UUID uuid(uuidStr);
		if (uuid == UUID::Invalid)
		{
			Log::Warn("SpriteAnimator_SetTextureUUID: invalid UUID {}", uuidStr);
			animator->SetTexture(std::shared_ptr<Texture>());
			return;
		}

		Metadata* meta = AssetRegistry::GetMetadata(uuid);
		if (!meta)
		{
			Log::Warn("SpriteAnimator_SetTextureUUID: texture metadata not found {}", uuidStr);
			animator->SetTexture(std::shared_ptr<Texture>());
			return;
		}

		auto tex = ResourceManager::GetTexture(*meta);
		if (tex && tex->Load())
			animator->SetTexture(tex);
		else
			Log::Warn("SpriteAnimator_SetTextureUUID: could not load texture {}", uuidStr);
	}

	static void SpriteAnimator_GetGrid(MonoString* entityID, MonoString* componentID, int* columns, int* rows)
	{
		if (columns) *columns = 1;
		if (rows) *rows = 1;

		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (!animator)
			return;
		ivec2 grid = animator->GetGrid();
		if (columns) *columns = grid.x;
		if (rows) *rows = grid.y;
	}

	static void SpriteAnimator_SetGrid(MonoString* entityID, MonoString* componentID, int columns, int rows)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (!animator)
			return;
		animator->SetGrid(ivec2(columns, rows));
	}

	static int SpriteAnimator_GetStartFrame(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetStartFrame() : 0;
	}

	static void SpriteAnimator_SetStartFrame(MonoString* entityID, MonoString* componentID, int frame)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetStartFrame(frame);
	}

	static int SpriteAnimator_GetFrameCount(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 1;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetFrameCount() : 1;
	}

	static void SpriteAnimator_SetFrameCount(MonoString* entityID, MonoString* componentID, int count)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetFrameCount(count);
	}

	static float SpriteAnimator_GetFPS(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0.0f;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetFPS() : 0.0f;
	}

	static void SpriteAnimator_SetFPS(MonoString* entityID, MonoString* componentID, float fps)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetFPS(fps);
	}

	static MonoBoolean SpriteAnimator_GetLoop(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetLoop() : false;
	}

	static void SpriteAnimator_SetLoop(MonoString* entityID, MonoString* componentID, MonoBoolean loop)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetLoop(loop != 0);
	}

	static MonoBoolean SpriteAnimator_GetPlayOnStart(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetPlayOnStart() : false;
	}

	static void SpriteAnimator_SetPlayOnStart(MonoString* entityID, MonoString* componentID, MonoBoolean playOnStart)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetPlayOnStart(playOnStart != 0);
	}

	static MonoBoolean SpriteAnimator_GetPlaying(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetPlaying() : false;
	}

	static void SpriteAnimator_SetPlaying(MonoString* entityID, MonoString* componentID, MonoBoolean playing)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetPlaying(playing != 0);
	}

	static void SpriteAnimator_Play(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->Play();
	}

	static void SpriteAnimator_Stop(MonoString* entityID, MonoString* componentID, MonoBoolean resetTime)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->Stop(resetTime != 0);
	}

	static void SpriteAnimator_SetCurrentFrame(MonoString* entityID, MonoString* componentID, int frameIndex)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		if (animator)
			animator->SetCurrentFrame(frameIndex);
	}

	static int SpriteAnimator_GetCurrentFrame(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		SpriteAnimator* animator = Utils::GetComponent<SpriteAnimator>(entity, componentID);
		return animator ? animator->GetCurrentFrame() : 0;
	}

#pragma endregion

#pragma region Camera
	static void Camera_SetFov(MonoString* entityID, MonoString* componentID, float fov) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			camera->SetFov(fov);
	}

	static float Camera_GetFov(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			return camera->GetFov();
		return 0;
	}

	static float Camera_GetNearPlane(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			return camera->GetNearPlane();
		return 0;
	}

	static void Camera_SetNearPlane(MonoString* entityID, MonoString* componentID, float nearPlane) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			camera->SetNearPlane(nearPlane);
	}

	static float Camera_GetFarPlane(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			return camera->GetFarPlane();
		return 0;
	}

	static void Camera_SetFarPlane(MonoString* entityID, MonoString* componentID, float farPlane) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			camera->SetFarPlane(farPlane);
	}

	static bool Camera_IsMainCamera(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			return camera->IsMainCamera();
		return false;
	}

	static void Camera_SetMainCamera(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			camera->SetAsMainCamera();
	}

	static MonoString* Camera_GetMainCamera() {
		Camera* camera = Camera::GetMainCamera();
		if (camera) {
			std::shared_ptr<Entity> entity = camera->GetOwner();
			return ScriptingManager::CreateString(entity->GetUUID().Get().c_str());
		}
		return ScriptingManager::CreateString("");
	}

	static void Camera_GetViewport(MonoString* entityID, MonoString* componentID,vec4* viewport) {
		*viewport = vec4(0);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			*viewport = camera->GetViewport();
	}

	static void Camera_SetOrthoSize(MonoString* entityID, MonoString* componentID, float size) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			camera->SetOrthoSize(size);
	}

	static float Camera_GetOrthoSize(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			return camera->GetOrthoSize();
		return 0;
	}

	static void Camera_SetProjection(MonoString* entityID, MonoString* componentID, int projectionType) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			camera->SetProjection((CameraProjection)projectionType);
	}

	static int Camera_GetProjection(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
			return (int)camera->GetProjection();
		return 0;
	}

	struct MonoRay
	{
		vec3 origin;
		vec3 direction;
		float length;
	};

	static void Camera_ScreenToWorldRay(MonoString* entityID, MonoString* componentID, vec2* screenPoint, MonoRay* outRay)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Camera* camera = Utils::GetComponent<Camera>(entity, componentID);
		if (camera)
		{
			Ray r = Camera::ScreenToWorldRay(*camera, screenPoint->x, screenPoint->y);
			outRay->origin = r.StartPoint();
			outRay->direction = r.Direction();
		}
		else {
			outRay->origin = { 0,0,0 };
			outRay->direction = { 0,0,0 };
		}
	}

#pragma endregion

#pragma region Input
	static MonoBoolean Input_IsKeyDown(int keycode)
	{
		return Application::GetInstance().GetInputEvent().GetKeyStatus((SDL_Scancode)keycode) == KeyState::DOWN;
	}

	static MonoBoolean Input_IsKeyUp(int keycode)
	{
		return Application::GetInstance().GetInputEvent().GetKeyStatus((SDL_Scancode)keycode) == KeyState::UP;
	}

	static MonoBoolean Input_IsKeyPressed(int keycode)
	{
		return Application::GetInstance().GetInputEvent().GetKeyStatus((SDL_Scancode)keycode) == KeyState::REPEAT;
	}

	static MonoBoolean Input_IsMouseButtonDown(int mouseButton)
	{
		return Application::GetInstance().GetInputEvent().GetMouseButtonStatus(mouseButton) == KeyState::DOWN;
	}

	static MonoBoolean Input_IsMouseButtonUp(int mouseButton)
	{
		return Application::GetInstance().GetInputEvent().GetMouseButtonStatus(mouseButton) == KeyState::UP;
	}

	static MonoBoolean Input_IsMouseButtonPressed(int mouseButton)
	{
		return Application::GetInstance().GetInputEvent().GetMouseButtonStatus(mouseButton) == KeyState::REPEAT;
	}
	static MonoBoolean Input_IsGamepadButtonDown(int gamepadButton)
	{
		return Application::GetInstance().GetInputEvent().GetGamepadButtonStatus((SDL_GamepadButton)gamepadButton) == KeyState::DOWN;
	}

	static MonoBoolean Input_IsGamepadButtonUp(int gamepadButton)
	{
		return Application::GetInstance().GetInputEvent().GetGamepadButtonStatus((SDL_GamepadButton)gamepadButton) == KeyState::UP;
	}

	static MonoBoolean Input_IsGamepadButtonPressed(int gamepadButton)
	{
		return Application::GetInstance().GetInputEvent().GetGamepadButtonStatus((SDL_GamepadButton)gamepadButton) == KeyState::REPEAT;
	}

	static void Input_GetMousePosition(vec2* mousePosition)
	{
		*mousePosition = Application::GetInstance().GetInputEvent().GetMousePosition();
	}

	static void Input_GetMouseDelta(vec2* mouseDelta)
	{
		*mouseDelta = Application::GetInstance().GetInputEvent().GetMouseDelta();
	}

	static void Input_GetScrollDelta(vec2* scrollDelta)
	{
		*scrollDelta = Application::GetInstance().GetInputEvent().GetScrollDelta();
	}

	static void Input_GetLeftAxis(vec2* axis) {
		*axis = Application::GetInstance().GetInputEvent().GetLeftAxis();
	}

	static void Input_GetRightAxis(vec2* axis) {
		*axis = Application::GetInstance().GetInputEvent().GetRightAxis();
	}

	static float Input_GetLeftTrigger() {
		return Application::GetInstance().GetInputEvent().GetLeftTrigger();
	}

	static float Input_GetRightTrigger() {
		return Application::GetInstance().GetInputEvent().GetRightTrigger();
	}


	static MonoBoolean Input_IsAnyKeyDown() {
		return Application::GetInstance().GetInputEvent().AnyKeyDown();
	}

	static MonoBoolean Input_IsAnyButtonDown() {
		return Application::GetInstance().GetInputEvent().AnyButtonDown();
	}

	static MonoBoolean Input_IsAnyMouseButtonDown() {
		return Application::GetInstance().GetInputEvent().AnyMouseButtonDown();
	}

	static MonoBoolean Input_IsAnyDown() {
		return Application::GetInstance().GetInputEvent().AnyDown();
	}


	static void Input_SetAxisDeadzone(float deadzone) {
		Application::GetInstance().GetInputEvent().SetAxisDeadzone(deadzone);
	}

	static float Input_GetAxisDeadzone() {
		return Application::GetInstance().GetInputEvent().GetAxisDeadzone();
	}



#pragma endregion

#pragma region Time
	static float Time_GetDeltaTime()
	{
		return (float)Time::GetDeltaTime();
	}

	static float Time_GetFixedDeltaTime()
	{
		return (float)Time::GetFixedDeltaTime();
	}

	static float Time_GetUnscaledDeltaTime()
	{
		return (float)Time::GetUnscaledDeltaTime();
	}

	static float Time_GetTimeScale()
	{
		return Time::GetTimeScale();
	}

	static void Time_SetTimeScale(float scale)
	{
		Time::SetTimeScale(scale);
	}
#pragma endregion

#pragma region BoxCollider

	static MonoString* BoxCollider_GetLayer(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
		{
			unsigned int tagIndex = collider->GetLayerIndex();
			const CollisionLayer& tag = CollisionProcessor::GetLayer(tagIndex);
			return ScriptingManager::CreateString(tag.name.c_str());
		}
		return ScriptingManager::CreateString("");
	}

	static void BoxCollider_SetLayer(MonoString* entityID, MonoString* componentID, MonoString* tagStr)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
		{
			std::string targetTag = Utils::MonoStringToString(tagStr);
			collider->SetLayer(targetTag);
		}
	}

	static void BoxCollider_SetLocalCenter(MonoString* entityID, MonoString* componentID, vec3* center)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->SetLocalCenter(*center);
	}

	static void BoxCollider_GetLocalCenter(MonoString* entityID, MonoString* componentID, vec3* outCenter)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			*outCenter =  collider->GetLocalCenter();
	}

	static void BoxCollider_SetLocalExtents(MonoString* entityID, MonoString* componentID, vec3* extents)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->SetLocalExtents(*extents);
	}

	static void BoxCollider_GetLocalExtents(MonoString* entityID, MonoString* componentID, vec3* outExtents)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			*outExtents = collider->GetLocalExtents();
	}

	static bool BoxCollider_IsColliding(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->IsColliding();
		return false;
	}

	static bool BoxCollider_HasCollided(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->CollidedThisFrame();
		return false;
	}

	static bool BoxCollider_HasEndedCollision(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->StoppedColliding();
		return false;
	}

	static bool BoxCollider_IsTrigger(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->IsTrigger();
		return false;
	}

	static void BoxCollider_SetTrigger(MonoString* entityID, MonoString* componentID, MonoBoolean isTrigger)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->SetIsTrigger(isTrigger !=0);
	}

	static bool BoxCollider_IsStatic(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->IsStatic();
		return false;
	}

	static void BoxCollider_SetStatic(MonoString* entityID, MonoString* componentID, MonoBoolean isStatic)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->SetIsStatic(isStatic != 0);
	}

	static void BoxCollider_SetIncludeMask(MonoString* entityID, MonoString* componentID, int includeMask) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->SetIncludeMask(includeMask);
	}

	static int BoxCollider_GetIncludeMask(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return -1;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->GetIncludeMask();
		return -1;
	}

	static void BoxCollider_AddIncludeMask(MonoString* entityID, MonoString* componentID, int includeMask) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->IncludeLayer(includeMask);
	}

	static void BoxCollider_RemoveIncludeMask(MonoString* entityID, MonoString* componentID, int includeMask) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->RemoveIncludedLayer(includeMask);
	}

	static void BoxCollider_SetExcludeMask(MonoString* entityID, MonoString* componentID, int excludeMask) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->SetExcludeMask(excludeMask);
	}

	static int BoxCollider_GetExcludeMask(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return -1;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			return collider->GetExcludeMask();
		return -1;
	}

	static void BoxCollider_AddExcludeMask(MonoString* entityID, MonoString* componentID, int excludeMask) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->ExcludeLayer(excludeMask);
	}

	static void BoxCollider_RemoveExcludeMask(MonoString* entityID, MonoString* componentID, int excludeMask) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		BoxCollider* collider = Utils::GetComponent<BoxCollider>(entity, componentID);
		if (collider)
			collider->RemoveExcludedLayer(excludeMask);
	}
#pragma endregion

#pragma region AudioSource
	static void AudioSource_Play(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->Play();
	}

	static void AudioSource_Stop(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->Stop();
	}

	static void AudioSource_SetLoop(MonoString* entityID, MonoString* componentID, MonoBoolean loop)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->SetLoop(loop!=0);
	}

	static void AudioSource_SetPitch(MonoString* entityID, MonoString* componentID, float pitch)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->SetPitch(pitch);
	}

	static void AudioSource_SetVolume(MonoString* entityID, MonoString* componentID, float volume)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->SetVolume(volume);
	}


	static void AudioSource_SetPan(MonoString* entityID, MonoString* componentID, float pan)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->SetPan(pan);
	}

	static void AudioSource_SetSet3DMinMaxDistance(MonoString* entityID, MonoString* componentID, float min, float max)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			audioSource->Set3DMinMaxDistance(min,max);
	}

	static MonoBoolean AudioSource_IsLooping(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			return audioSource->IsLooping();
		return false;
	}

	static float AudioSource_GetPitch(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			return audioSource->GetPitch();
		return 0;
	}

	static float AudioSource_GetVolume(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			return audioSource->GetVolume();
		return 0;
	}

	static float AudioSource_GetPan(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity, componentID);
		if (audioSource)
			return audioSource->GetPan();
		return 0;
	}

	static void AudioSource_GetSet3DMinMaxDistance(MonoString* entityID, MonoString* componentID, float* min, float* max)
	{
		*min = 0;
		*max = 0;
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity) {
			return;
		}
		AudioSource* audioSource = Utils::GetComponent<AudioSource>(entity,componentID);
		if (!audioSource) {
			return;
		}

		float minVal = 0;
		float maxVal = 0;
		audioSource->Get3DMinMaxDistance(minVal, maxVal);
		*min = minVal;
		*max = maxVal;
	}

#pragma endregion

#pragma region Collisions

	struct MonoRaycastHit
	{
		MonoString* entityID;
		MonoString* componentID;
		vec3 point;
		float distance;
	};

	static MonoBoolean Collisions_Raycast(vec3* origin, vec3* direction, float maxDistance, MonoRaycastHit* outHit, int layerMask, MonoString* entityToAvoidID, MonoString* componentToAvoidID)
	{
		Ray ray(*origin, *direction, maxDistance);

		std::shared_ptr<Entity> avoidEntity = Utils::GetEntityNoLog(entityToAvoidID);
		BoxCollider* avoidCollider = nullptr;
		if (avoidEntity)
		{
			avoidCollider = Utils::GetComponentNoLog<BoxCollider>(avoidEntity, componentToAvoidID);
		}

		RaycastHit nativeHit;

		if (!CollisionProcessor::Raycast(ray, nativeHit, layerMask, avoidCollider))
			return false;

		BoxCollider* collider = nativeHit.collider;
		if (!collider)
			return false;

		

		std::shared_ptr<Entity> entity = collider->GetOwner();
		UUID componentUUID = collider->GetUUID();

		outHit->entityID = ScriptingManager::CreateString(entity->GetUUID().Get().c_str());
		outHit->componentID = ScriptingManager::CreateString(componentUUID.Get().c_str());
		outHit->point = nativeHit.point;
		outHit->distance = nativeHit.distance;

		return true;
	}

	static int Collisions_GetLayerBit(MonoString* layerName)
	{
		std::string name = Utils::MonoStringToString(layerName);
		int index = CollisionProcessor::GetLayerIndex(name);

		if (index < 0 || index >= Loopie::MAX_LAYERS)
			return -1;

		return CollisionProcessor::GetLayer(index).bit;
	}

#pragma endregion

#pragma region Gizmo
	static void Gizmo_DrawLine(vec3* start, vec3* end, vec4* color) {
		Gizmo::DrawLine(*start, *end, *color);
	}
#pragma endregion

#pragma region Scene
	static bool Scene_LoadByID(MonoString* sceneID) {
		UUID uuid(Utils::MonoStringToString(sceneID));
		return Application::GetInstance().GetScene().RequestLoad(uuid);
	}
#pragma endregion

#pragma region ParticleSystem
	static void ParticleSystem_Play(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet)
			particleComponet->Play();
	}
	static void ParticleSystem_Stop(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet)
			particleComponet->Stop();
	}
	static bool ParticleSystem_IsPlaying(MonoString* entityID, MonoString* componentID) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet)
			return particleComponet->IsPlaying();
		return false;
	}

	static int ParticleSystem_GetEmitterIndex(MonoString* entityID, MonoString* componentID, MonoString* emitterName) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return-1;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			return particleComponet->GetEmitterIndexByName(Utils::MonoStringToString(emitterName));
		}
		return -1;
	}

	static void ParticleSystem_SetEmitterState(MonoString* entityID, MonoString* componentID, int emitterIndex, MonoBoolean activeState) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if(emitterIndex >=0 && emitterIndex < particleComponet->GetEmittersVector().size())
			particleComponet->GetEmittersVector()[emitterIndex]->SetActive(activeState!=0);
		}
	}
	static bool ParticleSystem_GetEmitterState(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetIsActive();
		}
		return false;
	}
	static void ParticleSystem_SetEmitterName(MonoString* entityID, MonoString* componentID, int emitterIndex, MonoString* emitterName) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetName(Utils::MonoStringToString(emitterName));
		}
	}
	static MonoString* ParticleSystem_GetEmitterName(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return ScriptingManager::CreateString(particleComponet->GetEmittersVector()[emitterIndex]->GetName().c_str());
		}
		return ScriptingManager::CreateString("");
	}
	static void ParticleSystem_SetEmitterSpawnRate(MonoString* entityID, MonoString* componentID, int emitterIndex, int spawnRate) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetSpawnRate(spawnRate);
		}
	}
	static int ParticleSystem_GetEmitterSpawnRate(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetSpawnrate();
		}
		return 0;
	}
	static void ParticleSystem_SetEmitterMaxParticles(MonoString* entityID, MonoString* componentID, int emitterIndex, int maxParticles) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetMaxParticles(maxParticles);
		}
	}
	static int ParticleSystem_GetEmitterMaxParticles(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetMaxParticles();
		}
		return 0;
	}
	static void ParticleSystem_SetEmitterPosition(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* position) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetPosition(*position);
		}
	}
	static void ParticleSystem_GetEmitterPosition(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* position) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*position = particleComponet->GetEmittersVector()[emitterIndex]->GetPosition();
		}
	}
	static void ParticleSystem_SetEmitterPositionOffset(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* positionOffset) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetPositionOffSet(*positionOffset);
		}
	}
	static void ParticleSystem_GetEmitterPositionOffset(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* positionOffset) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*positionOffset = particleComponet->GetEmittersVector()[emitterIndex]->GetPositionOffSet();
		}
	}
	static void ParticleSystem_SetEmitterFollowParent(MonoString* entityID, MonoString* componentID, int emitterIndex, MonoBoolean followParent) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetParticlesFollowEmitter(followParent != 0);
		}
	}
	static bool ParticleSystem_GetEmitterFollowParent(MonoString* entityID, MonoString* componentID, int emitterIndex, MonoBoolean followParent) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetParticlesFollowEmitter();
		}
		return false;
	}
	static void ParticleSystem_SetEmitterLocalVelocity(MonoString* entityID, MonoString* componentID, int emitterIndex, MonoBoolean localVelocity) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetLocalVelocity(localVelocity != 0);
		}
	}
	static bool ParticleSystem_GetEmitterLocalVelocity(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetLocalVelocity();
		}
		return false;
	}
	static void ParticleSystem_SetEmitterRotation(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* rotation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->SetEmitterRotation(quaternion(radians(*rotation)));
		}
	}
	static void ParticleSystem_GetEmitterRotation(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* rotation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size()) {
				quaternion emitterRot = particleComponet->GetEmittersVector()[emitterIndex]->GetEmitterRotation();
				*rotation = degrees(eulerAngles(emitterRot));
			}
		}
	}

	static void ParticleSystem_SetEmitterPropPosition(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* position) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().Position = *position;
		}
	}
	static void ParticleSystem_GetEmitterPropPosition(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* position) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*position = particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().Position;
		}
	}
	static void ParticleSystem_SetEmitterPropVelocity(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* velocity) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().Velocity = *velocity;
		}
	}
	static void ParticleSystem_GetEmitterPropVelocity(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* velocity) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*velocity = particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().Velocity;
		}
	}
	static void ParticleSystem_SetEmitterPropVelocityVariation(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* velocityVariation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().VelocityVariation = *velocityVariation;
		}
	}
	static void ParticleSystem_GetEmitterPropVelocityVariation(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* velocityVariation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*velocityVariation = particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().VelocityVariation;
		}
	}
	static void ParticleSystem_SetEmitterPropPositionVariation(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* positionVariation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().PositionVariation = *positionVariation;
		}
	}
	static void ParticleSystem_GetEmitterPropPositionVariation(MonoString* entityID, MonoString* componentID, int emitterIndex, vec3* positionVariation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*positionVariation = particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().PositionVariation;
		}
	}
	static void ParticleSystem_SetEmitterPropColorBegin(MonoString* entityID, MonoString* componentID, int emitterIndex, vec4* colorBegin) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().ColorBegin = *colorBegin;
		}
	}
	static void ParticleSystem_GetEmitterPropColorBegin(MonoString* entityID, MonoString* componentID, int emitterIndex, vec4* colorBegin) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*colorBegin = particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().ColorBegin;
		}
	}
	static void ParticleSystem_SetEmitterPropColorEnd(MonoString* entityID, MonoString* componentID, int emitterIndex, vec4* colorEnd) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().ColorEnd = *colorEnd;
		}
	}
	static void ParticleSystem_GetEmitterPropColorEnd(MonoString* entityID, MonoString* componentID, int emitterIndex, vec4* colorEnd) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				*colorEnd = particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().ColorEnd;
		}
	}
	static void ParticleSystem_SetEmitterPropSizeBegin(MonoString* entityID, MonoString* componentID, int emitterIndex, float sizeBegin) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().SizeBegin = sizeBegin;
		}
	}
	static float ParticleSystem_GetEmitterPropSizeBegin(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().SizeBegin;
		}
		return 0;
	}
	static void ParticleSystem_SetEmitterPropSizeEnd(MonoString* entityID, MonoString* componentID, int emitterIndex, float sizeEnd) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().SizeEnd = sizeEnd;
		}
	}
	static float ParticleSystem_GetEmitterPropSizeEnd(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().SizeEnd;
		}
		return 0;
	}
	static void ParticleSystem_SetEmitterSizeVariation(MonoString* entityID, MonoString* componentID, int emitterIndex, float sizeVariation) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().SizeVariation = sizeVariation;
		}
	}
	static float ParticleSystem_GetEmitterSizeVariation(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().SizeVariation;
		}
		return 0;
	}
	static void ParticleSystem_SetEmitterPropLifetime(MonoString* entityID, MonoString* componentID, int emitterIndex, float lifetime) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().LifeTime = lifetime;
		}
	}
	static float ParticleSystem_GetEmitterPropLifetime(MonoString* entityID, MonoString* componentID, int emitterIndex) {
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		ParticleComponent* particleComponet = Utils::GetComponent<ParticleComponent>(entity, componentID);
		if (particleComponet) {
			if (emitterIndex >= 0 && emitterIndex < particleComponet->GetEmittersVector().size())
				return particleComponet->GetEmittersVector()[emitterIndex]->GetEmissionProperties().LifeTime;
		}
		return 0;
	}
#pragma endregion

#pragma region UIElement
	static int UIElement_GetSortingLayer(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Component* component = Utils::GetComponent<Component>(entity, componentID);
		if (!component)
			return 0;
		UIElement* ui = component->AsUIElement();
		return ui ? ui->GetSortingLayer() : 0;
	}

	static void UIElement_SetSortingLayer(MonoString* entityID, MonoString* componentID, int layer)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Component* component = Utils::GetComponent<Component>(entity, componentID);
		if (!component)
			return;
		UIElement* ui = component->AsUIElement();
		if (ui)
			ui->SetSortingLayer(layer);
	}

	static int UIElement_GetOrderInLayer(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Component* component = Utils::GetComponent<Component>(entity, componentID);
		if (!component)
			return 0;
		UIElement* ui = component->AsUIElement();
		return ui ? ui->GetOrderInLayer() : 0;
	}

	static void UIElement_SetOrderInLayer(MonoString* entityID, MonoString* componentID, int order)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Component* component = Utils::GetComponent<Component>(entity, componentID);
		if (!component)
			return;
		UIElement* ui = component->AsUIElement();
		if (ui)
			ui->SetOrderInLayer(order);
	}
#pragma endregion

#pragma region Image
	static void Image_GetTint(MonoString* entityID, MonoString* componentID, vec4* outTint)
	{
		*outTint = vec4(1.0f);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Image* image = Utils::GetComponent<Image>(entity, componentID);
		if (!image)
			return;
		*outTint = image->GetTint();
	}

	static void Image_SetTint(MonoString* entityID, MonoString* componentID, vec4* tint)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Image* image = Utils::GetComponent<Image>(entity, componentID);
		if (!image)
			return;
		image->SetTint(*tint);
	}
#pragma endregion

#pragma region Text
	static void Text_GetColor(MonoString* entityID, MonoString* componentID, vec4* outColor)
	{
		*outColor = vec4(1.0f);

		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;

		Text* text = Utils::GetComponent<Text>(entity, componentID);
		if (!text)
			return;

		*outColor = text->GetColor();
	}

	static void Text_SetColor(MonoString* entityID, MonoString* componentID, vec4* color)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;

		Text* text = Utils::GetComponent<Text>(entity, componentID);
		if (!text)
			return;

		text->SetColor(*color);	
	}
	static MonoString* Text_GetText(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Text* text = Utils::GetComponent<Text>(entity, componentID);
		if (text) {
			return ScriptingManager::CreateString(text->GetText().c_str());
		}
		return ScriptingManager::CreateString("");
	}

	static void Text_SetText(MonoString* entityID, MonoString* componentID, MonoString* textValue)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Text* text = Utils::GetComponent<Text>(entity, componentID);
		if (text) {
			text->SetText(Utils::MonoStringToString(textValue));
		}
	}
#pragma endregion

#pragma region Button
	static MonoBoolean Button_IsInteractable(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		return button ? button->IsInteractable() : false;
	}

	static void Button_SetInteractable(MonoString* entityID, MonoString* componentID, MonoBoolean interactable)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			button->SetInteractable(interactable != 0);
	}

	static MonoBoolean Button_IsHovered(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		return button ? button->IsHovered() : false;
	}

	static MonoBoolean Button_IsPressed(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		return button ? button->IsPressed() : false;
	}

	static void Button_TriggerClick(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			button->TriggerClick();
	}

	static int Button_GetTransitionMode(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return 0;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		return button ? (int)button->GetTransitionMode() : 0;
	}

	static void Button_SetTransitionMode(MonoString* entityID, MonoString* componentID, int mode)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return;
		mode = std::clamp(mode, 0, 1);
		button->SetTransitionMode((Button::VisualTransitionMode)mode);
	}

	static void Button_GetNormalColor(MonoString* entityID, MonoString* componentID, vec4* outColor)
	{
		*outColor = vec4(1.0f);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			*outColor = button->GetNormalColor();
	}

	static void Button_SetNormalColor(MonoString* entityID, MonoString* componentID, vec4* color)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			button->SetNormalColor(*color);
	}

	static void Button_GetHoveredColor(MonoString* entityID, MonoString* componentID, vec4* outColor)
	{
		*outColor = vec4(1.0f);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			*outColor = button->GetHoveredColor();
	}

	static void Button_SetHoveredColor(MonoString* entityID, MonoString* componentID, vec4* color)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			button->SetHoveredColor(*color);
	}

	static void Button_GetPressedColor(MonoString* entityID, MonoString* componentID, vec4* outColor)
	{
		*outColor = vec4(1.0f);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			*outColor = button->GetPressedColor();
	}

	static void Button_SetPressedColor(MonoString* entityID, MonoString* componentID, vec4* color)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			button->SetPressedColor(*color);
	}

	static void Button_GetDisabledColor(MonoString* entityID, MonoString* componentID, vec4* outColor)
	{
		*outColor = vec4(1.0f);
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			*outColor = button->GetDisabledColor();
	}

	static void Button_SetDisabledColor(MonoString* entityID, MonoString* componentID, vec4* color)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (button)
			button->SetDisabledColor(*color);
	}

	static MonoString* Button_GetTextureUUID(const std::shared_ptr<Texture>& tex)
	{
		if (!tex)
			return ScriptingManager::CreateString("");
		return ScriptingManager::CreateString(tex->GetUUID().Get().c_str());
	}

	static std::shared_ptr<Texture> Button_LoadTextureFromUUID(MonoString* uuidStr)
	{
		if (!uuidStr)
			return std::shared_ptr<Texture>();
		std::string uuidString = Utils::MonoStringToString(uuidStr);
		if (uuidString.empty())
			return std::shared_ptr<Texture>();
		UUID uuid(uuidString);
		if (uuid == UUID::Invalid)
			return std::shared_ptr<Texture>();
		Metadata* meta = AssetRegistry::GetMetadata(uuid);
		if (!meta)
			return std::shared_ptr<Texture>();
		auto tex = ResourceManager::GetTexture(*meta);
		if (tex)
			tex->Load();
		return tex;
	}

	static MonoString* Button_GetNormalTextureUUID(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return ScriptingManager::CreateString("");
		return Button_GetTextureUUID(button->GetNormalTexture());
	}

	static void Button_SetNormalTextureUUID(MonoString* entityID, MonoString* componentID, MonoString* textureUUID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return;
		button->SetNormalTexture(Button_LoadTextureFromUUID(textureUUID));
	}

	static MonoString* Button_GetHoveredTextureUUID(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return ScriptingManager::CreateString("");
		return Button_GetTextureUUID(button->GetHoveredTexture());
	}

	static void Button_SetHoveredTextureUUID(MonoString* entityID, MonoString* componentID, MonoString* textureUUID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return;
		button->SetHoveredTexture(Button_LoadTextureFromUUID(textureUUID));
	}

	static MonoString* Button_GetPressedTextureUUID(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return ScriptingManager::CreateString("");
		return Button_GetTextureUUID(button->GetPressedTexture());
	}

	static void Button_SetPressedTextureUUID(MonoString* entityID, MonoString* componentID, MonoString* textureUUID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return;
		button->SetPressedTexture(Button_LoadTextureFromUUID(textureUUID));
	}

	static MonoString* Button_GetDisabledTextureUUID(MonoString* entityID, MonoString* componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return ScriptingManager::CreateString("");
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return ScriptingManager::CreateString("");
		return Button_GetTextureUUID(button->GetDisabledTexture());
	}

	static void Button_SetDisabledTextureUUID(MonoString* entityID, MonoString* componentID, MonoString* textureUUID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Button* button = Utils::GetComponent<Button>(entity, componentID);
		if (!button)
			return;
		button->SetDisabledTexture(Button_LoadTextureFromUUID(textureUUID));
	}

#pragma endregion

#pragma region Window
	static void Window_SetTargetFramerate(int targetFramerate) {
		Window& window = Application::GetInstance().GetWindow();
		window.SetFramerateLimit(targetFramerate);
	}

	static int Window_GetTargetFramerate() {
		const Window& window = Application::GetInstance().GetWindow();
		return window.GetFramerateLimit();
	}

	static void Window_SetVSync(bool enabled) {
		Window& window = Application::GetInstance().GetWindow();
		window.SetVsync(enabled);
	}

	static bool Window_GetVSync() {
		const Window& window = Application::GetInstance().GetWindow();
		return window.IsVsyncEnabled();
	}

	static void Window_SetFullscreen(bool fullscreen) {
		Window& window = Application::GetInstance().GetWindow();
		window.SetWindowFullscreen(fullscreen);
	}

	static bool Window_GetFullscreen() {
		const Window& window = Application::GetInstance().GetWindow();
		return window.IsFullscreen();
	}

	static void Window_SetResizable(bool resizable) {
		Window& window = Application::GetInstance().GetWindow();
		window.SetResizable(resizable);
	}

	static void Window_SetSize(vec2* size) {
		Window& window = Application::GetInstance().GetWindow();
		window.SetWindowSize(size->x, size->y,false);
	}

	static void Window_GetSize(vec2* size) {
		Window& window = Application::GetInstance().GetWindow();
		*size = window.GetSize();
	}

#pragma Application

	static void Application_Quit() {
		Application::GetInstance().Quit();
	}

#pragma endregion

#pragma AudioMixer

	static void AudioMixer_SetBusVolume(MonoString* busName, float volume) {
		AudioManager::SetBusVolume(Utils::MonoStringToString(busName), volume);
	}

	static float AudioMixer_GetBusVolume(MonoString* busName) {
		return AudioManager::GetBusVolume(Utils::MonoStringToString(busName));
	}

#pragma endregion

	template<typename Comp, typename = std::enable_if_t<std::is_base_of_v<Component, Comp>>>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = Comp::GetIdentificableName();
				std::string managedTypename = fmt::format("Loopie.{}", typeName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptingManager::s_Data.CoreImage);
				if (!managedType)
				{
					Log::Warn("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](std::shared_ptr<Entity> entity) { return entity->HasComponent<Comp>(); };
				s_EntityGetComponentFuncs[managedType] = [](std::shared_ptr<Entity> entity, int index) { 
					Comp* compo = entity->GetComponent<Comp>(index);
					return compo ? compo->GetUUID() : UUID::Invalid; };
			}());
	}


	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		s_EntityGetComponentFuncs.clear();
		RegisterComponent<Transform>();
		RegisterComponent<Animator>();
		RegisterComponent<SpriteAnimator>();
		RegisterComponent<Camera>();
		RegisterComponent<MeshRenderer>();
		RegisterComponent<BoxCollider>();
		RegisterComponent<AudioSource>();
		RegisterComponent<AudioListener>();
		RegisterComponent<ParticleComponent>();
		RegisterComponent<Image>();
		RegisterComponent<Text>();
		RegisterComponent<Button>();
	}


	void ScriptGlue::RegisterFunctions()
	{
		ADD_INTERNAL_CALL(NativeLog);
		ADD_INTERNAL_CALL(NativeLog_Int);
		ADD_INTERNAL_CALL(NativeLog_Float);
		ADD_INTERNAL_CALL(NativeLog_Vector2);
		ADD_INTERNAL_CALL(NativeLog_Vector3);

		ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		ADD_INTERNAL_CALL(Entity_Create);
		ADD_INTERNAL_CALL(Entity_Clone);
		ADD_INTERNAL_CALL(Entity_Destroy);
		ADD_INTERNAL_CALL(Entity_AddComponent);
		ADD_INTERNAL_CALL(Entity_FindEntityByName);
		ADD_INTERNAL_CALL(Entity_FindEntityByID);
		ADD_INTERNAL_CALL(Entity_HasComponent);
		ADD_INTERNAL_CALL(Entity_GetComponent);
		ADD_INTERNAL_CALL(Entity_SetActive);
		ADD_INTERNAL_CALL(Entity_IsActive);
		ADD_INTERNAL_CALL(Entity_IsActiveInHierarchy);
		ADD_INTERNAL_CALL(Entity_GetParent);
		ADD_INTERNAL_CALL(Entity_SetParent);
		ADD_INTERNAL_CALL(Entity_GetName);
		ADD_INTERNAL_CALL(Entity_SetName);
		ADD_INTERNAL_CALL(Entity_GetChildCount);
		ADD_INTERNAL_CALL(Entity_GetChild);
		ADD_INTERNAL_CALL(Entity_GetChildByName);

		ADD_INTERNAL_CALL(Component_SetActive);
		ADD_INTERNAL_CALL(Component_IsActive);

		ADD_INTERNAL_CALL(Transform_GetPosition);
		ADD_INTERNAL_CALL(Transform_SetPosition);
		ADD_INTERNAL_CALL(Transform_GetLocalPosition);
		ADD_INTERNAL_CALL(Transform_SetLocalPosition);
		ADD_INTERNAL_CALL(Transform_GetRotation);
		ADD_INTERNAL_CALL(Transform_SetRotation);
		ADD_INTERNAL_CALL(Transform_GetLocalRotation);
		ADD_INTERNAL_CALL(Transform_SetLocalRotation);
		ADD_INTERNAL_CALL(Transform_GetLocalScale);
		ADD_INTERNAL_CALL(Transform_SetLocalScale);
		ADD_INTERNAL_CALL(Transform_Translate);
		ADD_INTERNAL_CALL(Transform_Rotate);
		ADD_INTERNAL_CALL(Transform_LookAt);
		ADD_INTERNAL_CALL(Transform_Forward);
		ADD_INTERNAL_CALL(Transform_Back);
		ADD_INTERNAL_CALL(Transform_Up);
		ADD_INTERNAL_CALL(Transform_Down);
		ADD_INTERNAL_CALL(Transform_Left);
		ADD_INTERNAL_CALL(Transform_Right);

		ADD_INTERNAL_CALL(Animator_Stop);
		ADD_INTERNAL_CALL(Animator_PlayClip);
		ADD_INTERNAL_CALL(Animator_Play);
		ADD_INTERNAL_CALL(Animator_Pause);
		ADD_INTERNAL_CALL(Animator_Resume);
		ADD_INTERNAL_CALL(Animator_GetCurrentClipIndex);
		ADD_INTERNAL_CALL(Animator_GetCurrentClipName);
		ADD_INTERNAL_CALL(Animator_GetCurrentClipDuration);
		ADD_INTERNAL_CALL(Animator_GetNextClipIndex);
		ADD_INTERNAL_CALL(Animator_GetNextClipName);
		ADD_INTERNAL_CALL(Animator_GetNextClipDuration);
		ADD_INTERNAL_CALL(Animator_GetClipName);
		ADD_INTERNAL_CALL(Animator_GetClipIndex);
		ADD_INTERNAL_CALL(Animator_GetClipDurationByIndex);
		ADD_INTERNAL_CALL(Animator_GetClipDurationByName);
		ADD_INTERNAL_CALL(Animator_GetPlaybackSpeed);
		ADD_INTERNAL_CALL(Animator_SetPlaybackSpeed);
		ADD_INTERNAL_CALL(Animator_SetLooping);
		ADD_INTERNAL_CALL(Animator_IsLooping);
		ADD_INTERNAL_CALL(Animator_IsPlaying);
		ADD_INTERNAL_CALL(Animator_IsInTransition);
		ADD_INTERNAL_CALL(Animator_GetCurrentTime);

		ADD_INTERNAL_CALL(SpriteAnimator_GetTextureUUID);
		ADD_INTERNAL_CALL(SpriteAnimator_SetTextureUUID);
		ADD_INTERNAL_CALL(SpriteAnimator_GetGrid);
		ADD_INTERNAL_CALL(SpriteAnimator_SetGrid);
		ADD_INTERNAL_CALL(SpriteAnimator_GetStartFrame);
		ADD_INTERNAL_CALL(SpriteAnimator_SetStartFrame);
		ADD_INTERNAL_CALL(SpriteAnimator_GetFrameCount);
		ADD_INTERNAL_CALL(SpriteAnimator_SetFrameCount);
		ADD_INTERNAL_CALL(SpriteAnimator_GetFPS);
		ADD_INTERNAL_CALL(SpriteAnimator_SetFPS);
		ADD_INTERNAL_CALL(SpriteAnimator_GetLoop);
		ADD_INTERNAL_CALL(SpriteAnimator_SetLoop);
		ADD_INTERNAL_CALL(SpriteAnimator_GetPlayOnStart);
		ADD_INTERNAL_CALL(SpriteAnimator_SetPlayOnStart);
		ADD_INTERNAL_CALL(SpriteAnimator_GetPlaying);
		ADD_INTERNAL_CALL(SpriteAnimator_SetPlaying);
		ADD_INTERNAL_CALL(SpriteAnimator_Play);
		ADD_INTERNAL_CALL(SpriteAnimator_Stop);
		ADD_INTERNAL_CALL(SpriteAnimator_GetCurrentFrame);
		ADD_INTERNAL_CALL(SpriteAnimator_SetCurrentFrame);

		ADD_INTERNAL_CALL(Camera_SetFov);
		ADD_INTERNAL_CALL(Camera_GetFov);
		ADD_INTERNAL_CALL(Camera_SetNearPlane);
		ADD_INTERNAL_CALL(Camera_GetNearPlane);
		ADD_INTERNAL_CALL(Camera_SetFarPlane);
		ADD_INTERNAL_CALL(Camera_GetFarPlane);
		ADD_INTERNAL_CALL(Camera_IsMainCamera);
		ADD_INTERNAL_CALL(Camera_SetMainCamera);
		ADD_INTERNAL_CALL(Camera_GetMainCamera);
		ADD_INTERNAL_CALL(Camera_GetViewport);
		ADD_INTERNAL_CALL(Camera_SetOrthoSize);
		ADD_INTERNAL_CALL(Camera_GetOrthoSize);
		ADD_INTERNAL_CALL(Camera_SetProjection);
		ADD_INTERNAL_CALL(Camera_GetProjection);
		ADD_INTERNAL_CALL(Camera_ScreenToWorldRay);

		ADD_INTERNAL_CALL(Input_IsKeyDown);
		ADD_INTERNAL_CALL(Input_IsKeyUp);
		ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ADD_INTERNAL_CALL(Input_IsMouseButtonDown);
		ADD_INTERNAL_CALL(Input_IsMouseButtonUp);
		ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		ADD_INTERNAL_CALL(Input_IsGamepadButtonDown);
		ADD_INTERNAL_CALL(Input_IsGamepadButtonUp);
		ADD_INTERNAL_CALL(Input_IsGamepadButtonPressed);
		ADD_INTERNAL_CALL(Input_GetMousePosition);
		ADD_INTERNAL_CALL(Input_GetMouseDelta);
		ADD_INTERNAL_CALL(Input_GetScrollDelta);
		ADD_INTERNAL_CALL(Input_GetLeftAxis);
		ADD_INTERNAL_CALL(Input_GetRightAxis);
		ADD_INTERNAL_CALL(Input_GetLeftTrigger);
		ADD_INTERNAL_CALL(Input_GetRightTrigger);
		ADD_INTERNAL_CALL(Input_IsAnyKeyDown);
		ADD_INTERNAL_CALL(Input_IsAnyButtonDown);
		ADD_INTERNAL_CALL(Input_IsAnyMouseButtonDown);
		ADD_INTERNAL_CALL(Input_IsAnyDown);
		ADD_INTERNAL_CALL(Input_SetAxisDeadzone);
		ADD_INTERNAL_CALL(Input_GetAxisDeadzone);

		ADD_INTERNAL_CALL(Time_GetDeltaTime);
		ADD_INTERNAL_CALL(Time_GetFixedDeltaTime);
		ADD_INTERNAL_CALL(Time_GetUnscaledDeltaTime);
		ADD_INTERNAL_CALL(Time_GetTimeScale);
		ADD_INTERNAL_CALL(Time_SetTimeScale);

		ADD_INTERNAL_CALL(BoxCollider_GetLayer);
		ADD_INTERNAL_CALL(BoxCollider_SetLayer);
		ADD_INTERNAL_CALL(BoxCollider_GetLocalCenter);
		ADD_INTERNAL_CALL(BoxCollider_SetLocalCenter);
		ADD_INTERNAL_CALL(BoxCollider_GetLocalExtents);
		ADD_INTERNAL_CALL(BoxCollider_SetLocalExtents);
		ADD_INTERNAL_CALL(BoxCollider_IsColliding);
		ADD_INTERNAL_CALL(BoxCollider_HasCollided);
		ADD_INTERNAL_CALL(BoxCollider_HasEndedCollision);
		ADD_INTERNAL_CALL(BoxCollider_IsStatic);
		ADD_INTERNAL_CALL(BoxCollider_SetStatic);
		ADD_INTERNAL_CALL(BoxCollider_IsTrigger);
		ADD_INTERNAL_CALL(BoxCollider_SetTrigger);
		ADD_INTERNAL_CALL(BoxCollider_SetIncludeMask);
		ADD_INTERNAL_CALL(BoxCollider_GetIncludeMask);
		ADD_INTERNAL_CALL(BoxCollider_AddIncludeMask);
		ADD_INTERNAL_CALL(BoxCollider_RemoveIncludeMask);
		ADD_INTERNAL_CALL(BoxCollider_SetExcludeMask);
		ADD_INTERNAL_CALL(BoxCollider_GetExcludeMask);
		ADD_INTERNAL_CALL(BoxCollider_AddExcludeMask);
		ADD_INTERNAL_CALL(BoxCollider_RemoveExcludeMask);

		ADD_INTERNAL_CALL(AudioSource_Play);
		ADD_INTERNAL_CALL(AudioSource_Stop);
		ADD_INTERNAL_CALL(AudioSource_SetLoop);
		ADD_INTERNAL_CALL(AudioSource_SetPitch);
		ADD_INTERNAL_CALL(AudioSource_SetVolume);
		ADD_INTERNAL_CALL(AudioSource_SetPan);
		ADD_INTERNAL_CALL(AudioSource_SetSet3DMinMaxDistance);

		ADD_INTERNAL_CALL(AudioSource_IsLooping);
		ADD_INTERNAL_CALL(AudioSource_GetPitch);
		ADD_INTERNAL_CALL(AudioSource_GetVolume);
		ADD_INTERNAL_CALL(AudioSource_GetPan);
		ADD_INTERNAL_CALL(AudioSource_GetSet3DMinMaxDistance);

		ADD_INTERNAL_CALL(Collisions_Raycast);
		ADD_INTERNAL_CALL(Collisions_GetLayerBit);

		ADD_INTERNAL_CALL(Gizmo_DrawLine);

		ADD_INTERNAL_CALL(Scene_LoadByID);

		ADD_INTERNAL_CALL(ParticleSystem_Play);
		ADD_INTERNAL_CALL(ParticleSystem_Stop);
		ADD_INTERNAL_CALL(ParticleSystem_IsPlaying);

		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterIndex);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterState);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterState);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterName);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterName);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterSpawnRate);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterSpawnRate);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterMaxParticles);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterMaxParticles);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPosition);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPosition);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPositionOffset);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPositionOffset);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterFollowParent);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterFollowParent);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterLocalVelocity);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterLocalVelocity);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterRotation);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterRotation);		
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropPosition);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropPosition);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropVelocity);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropVelocity);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropVelocityVariation);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropVelocityVariation);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropPositionVariation);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropPositionVariation);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropColorBegin);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropColorBegin);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropColorEnd);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropColorEnd);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropSizeBegin);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropSizeBegin);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropSizeEnd);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropSizeEnd);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterSizeVariation);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterSizeVariation);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmitterPropLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_GetEmitterPropLifetime);





		ADD_INTERNAL_CALL(UIElement_GetSortingLayer);
		ADD_INTERNAL_CALL(UIElement_SetSortingLayer);
		ADD_INTERNAL_CALL(UIElement_GetOrderInLayer);
		ADD_INTERNAL_CALL(UIElement_SetOrderInLayer);

		ADD_INTERNAL_CALL(Image_GetTint);
		ADD_INTERNAL_CALL(Image_SetTint);

		ADD_INTERNAL_CALL(Text_GetColor);
		ADD_INTERNAL_CALL(Text_SetColor);
		ADD_INTERNAL_CALL(Text_GetText);
		ADD_INTERNAL_CALL(Text_SetText);

		ADD_INTERNAL_CALL(Button_IsInteractable);
		ADD_INTERNAL_CALL(Button_SetInteractable);
		ADD_INTERNAL_CALL(Button_IsHovered);
		ADD_INTERNAL_CALL(Button_IsPressed);
		ADD_INTERNAL_CALL(Button_TriggerClick);
		ADD_INTERNAL_CALL(Button_GetTransitionMode);
		ADD_INTERNAL_CALL(Button_SetTransitionMode);
		ADD_INTERNAL_CALL(Button_GetNormalColor);
		ADD_INTERNAL_CALL(Button_SetNormalColor);
		ADD_INTERNAL_CALL(Button_GetHoveredColor);
		ADD_INTERNAL_CALL(Button_SetHoveredColor);
		ADD_INTERNAL_CALL(Button_GetPressedColor);
		ADD_INTERNAL_CALL(Button_SetPressedColor);
		ADD_INTERNAL_CALL(Button_GetDisabledColor);
		ADD_INTERNAL_CALL(Button_SetDisabledColor);
		ADD_INTERNAL_CALL(Button_GetNormalTextureUUID);
		ADD_INTERNAL_CALL(Button_SetNormalTextureUUID);
		ADD_INTERNAL_CALL(Button_GetHoveredTextureUUID);
		ADD_INTERNAL_CALL(Button_SetHoveredTextureUUID);
		ADD_INTERNAL_CALL(Button_GetPressedTextureUUID);
		ADD_INTERNAL_CALL(Button_SetPressedTextureUUID);
		ADD_INTERNAL_CALL(Button_GetDisabledTextureUUID);
		ADD_INTERNAL_CALL(Button_SetDisabledTextureUUID);

		ADD_INTERNAL_CALL(Window_SetTargetFramerate);
		ADD_INTERNAL_CALL(Window_GetTargetFramerate);
		ADD_INTERNAL_CALL(Window_SetVSync);
		ADD_INTERNAL_CALL(Window_GetVSync);
		ADD_INTERNAL_CALL(Window_SetFullscreen);
		ADD_INTERNAL_CALL(Window_GetFullscreen);
		ADD_INTERNAL_CALL(Window_SetResizable);
		ADD_INTERNAL_CALL(Window_SetSize);
		ADD_INTERNAL_CALL(Window_GetSize);

		ADD_INTERNAL_CALL(Application_Quit);

		ADD_INTERNAL_CALL(AudioMixer_SetBusVolume);
		ADD_INTERNAL_CALL(AudioMixer_GetBusVolume);
	}
}