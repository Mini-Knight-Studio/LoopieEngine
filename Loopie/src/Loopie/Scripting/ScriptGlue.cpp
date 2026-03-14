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

#include "Loopie/Core/UUID.h"
#include "Loopie/Core/InputEventManager.h"

#include "Loopie/Collisions/CollisionProcessor.h"

#include "Loopie/Render/Gizmo.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Assert.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

namespace Loopie
{
	static std::unordered_map<_MonoType*, std::function<bool(std::shared_ptr<Entity>)>> s_EntityHasComponentFuncs;
	static std::unordered_map<_MonoType*, std::function<UUID(std::shared_ptr<Entity>)>> s_EntityGetComponentFuncs;

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
	static MonoObject* Entity_GetScriptInstance(MonoString* entityID, MonoString* componentFullName)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if(!entity)
			return nullptr;

		std::vector<ScriptClass*> scriptComponents = entity->GetComponents<ScriptClass>();

		std::string compFullName = Utils::MonoStringToString(componentFullName);
		for (ScriptClass* script : scriptComponents)
		{
			if (!script || !script->GetScriptingClass())
				continue;

			if (script->IsSameType(compFullName))
			{
				return script->GetInstance();
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
			return nullptr;

		std::shared_ptr<Entity> parent = nullptr;
		if (parentID !=nullptr)
		{
			UUID parentUuid(Utils::MonoStringToString(parentID));
			if (parentUuid == UUID::Invalid) {
				Log::Warn("Invalid UUID: {}", parentUuid.Get());
				return nullptr;
			}
			parent = scene->GetEntity(parentUuid);
			if (!parent) {
				Log::Warn("Parent entity {} not found", parentUuid.Get());
				return nullptr;
			}
		}

		std::string name = Utils::MonoStringToString(entityName);
		std::shared_ptr<Entity> entity = scene->CreateEntity(name, parent);
		if(!entity)
			return nullptr;

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
			return nullptr;

		std::shared_ptr<Entity> clone = Utils::GetScene()->CloneEntity(entity, nullptr, (cloneChilds != 0));
		if (!clone)
			return nullptr;

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

	static MonoBoolean Entity_GetComponent(MonoString* entityID, MonoReflectionType* componentType, MonoString** componentID)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return false;

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		if (s_EntityHasComponentFuncs.find(managedType) == s_EntityHasComponentFuncs.end())
			return false;
		UUID componentUUID = s_EntityGetComponentFuncs.at(managedType)(entity);
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

	static void Animator_PlayClip(MonoString* entityID, MonoString* componentID, MonoString* clipName)
	{
		std::shared_ptr<Entity> entity = Utils::GetEntity(entityID);
		if (!entity)
			return;
		Animator* animator = Utils::GetComponent<Animator>(entity, componentID);
		if (animator) {
			std::string name = Utils::MonoStringToString(clipName);
			animator->Play(name);
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

	static MonoBoolean Collisions_Raycast(vec3* origin, vec3* direction, float maxDistance, MonoRaycastHit* outHit, int layerMask)
	{
		Ray ray(*origin, *direction, maxDistance);

		RaycastHit nativeHit;

		if (!CollisionProcessor::Raycast(ray, nativeHit, layerMask))
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
				s_EntityGetComponentFuncs[managedType] = [](std::shared_ptr<Entity> entity) { 
					Comp* compo = entity->GetComponent<Comp>(); 
					return compo ? compo->GetUUID() : UUID::Invalid; };
			}());
	}


	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		s_EntityGetComponentFuncs.clear();
		RegisterComponent<Transform>();
		RegisterComponent<Animator>();
		RegisterComponent<Camera>();
		RegisterComponent<MeshRenderer>();
		RegisterComponent<BoxCollider>();
		RegisterComponent<AudioSource>();
		RegisterComponent<AudioListener>();
		RegisterComponent<Image>();
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
		ADD_INTERNAL_CALL(Animator_GetClipName);
		ADD_INTERNAL_CALL(Animator_GetPlaybackSpeed);
		ADD_INTERNAL_CALL(Animator_SetPlaybackSpeed);
		ADD_INTERNAL_CALL(Animator_SetLooping);
		ADD_INTERNAL_CALL(Animator_IsLooping);
		ADD_INTERNAL_CALL(Animator_IsPlaying);
		ADD_INTERNAL_CALL(Animator_GetCurrentTime);

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
		ADD_INTERNAL_CALL(Input_IsAnyKeyDown);
		ADD_INTERNAL_CALL(Input_IsAnyButtonDown);
		ADD_INTERNAL_CALL(Input_IsAnyMouseButtonDown);
		ADD_INTERNAL_CALL(Input_IsAnyDown);
		ADD_INTERNAL_CALL(Input_SetAxisDeadzone);
		ADD_INTERNAL_CALL(Input_GetAxisDeadzone);

		ADD_INTERNAL_CALL(Time_GetDeltaTime);
		ADD_INTERNAL_CALL(Time_GetFixedDeltaTime);
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
		ADD_INTERNAL_CALL(Image_GetTint);
		ADD_INTERNAL_CALL(Image_SetTint);
	}
}