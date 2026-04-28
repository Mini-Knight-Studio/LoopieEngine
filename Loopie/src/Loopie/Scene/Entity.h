#pragma once

#include "Loopie/Core/UUID.h"
#include "Loopie/Core/IIdentificable.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Loopie {
	class Component;
	class Transform;
	class Scene;


	/// Maybe Add a CopyComponent
	class Entity : public std::enable_shared_from_this<Entity>
	{
	public:
		Entity(const std::string& name);
		~Entity();

		template<typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		T* AddComponent(Args&&... args)
		{
			if constexpr (std::is_same_v<T, Transform>) {
				if (m_transform && m_transform->GetTypeID() == T::GetTypeIDStatic())
					return static_cast<T*>(m_transform);
			}

			m_components.push_back(std::make_unique<T>(std::forward<Args>(args)...));
			T* componentPtr = static_cast<T*>(m_components.back().get());

			componentPtr->m_owner = weak_from_this();
			componentPtr->Init();

			if constexpr (std::is_same_v<T, Transform>) {
				m_transform = componentPtr;
			}

			m_componentsByUUID[componentPtr->GetUUID()] = componentPtr;
			m_componentsByType[T::GetTypeIDStatic()].push_back(componentPtr);

			m_transform->MarkHasChangedThisFrame();
			return componentPtr;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Transform, T>>>
		T* ReplaceTransform()
		{
			Transform* oldTransform = m_transform;
			if (!oldTransform)
				return AddComponent<T>();

			glm::vec3 position = oldTransform->GetLocalPosition();
			glm::quat rotation = oldTransform->GetLocalRotation();
			glm::vec3 scale = oldTransform->GetLocalScale();

			T* newTransform = AddComponent<T>();

			newTransform->SetLocalPosition(position);
			newTransform->SetLocalRotation(rotation);
			newTransform->SetLocalScale(scale);

			m_transform = newTransform;

			RemoveComponent(oldTransform);

			return newTransform;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		T* GetComponent() const
		{
			auto it = m_componentsByType.find(T::GetTypeIDStatic());
			if (it != m_componentsByType.end() && !it->second.empty())
				return static_cast<T*>(it->second.front());

			return nullptr;
		}


		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		T* GetComponent(UUID uuid) {
			auto it = m_componentsByUUID.find(uuid);
			if (it != m_componentsByUUID.end())
				return static_cast<T*>(it->second);
			return nullptr;
		}

		Component* GetComponent(UUID uuid);

		void OnComponentUUIDChange(Component* component, UUID oldUUID);

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		std::vector<T*> GetComponents() const
		{
			std::vector<T*> result;

			auto it = m_componentsByType.find(T::GetTypeIDStatic());
			if (it == m_componentsByType.end())
				return result;

			result.reserve(it->second.size());

			for (auto* c : it->second)
				result.push_back(static_cast<T*>(c));

			return result;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		const std::vector<Component*>* GetComponentsRaw() const
		{
			auto it = m_componentsByType.find(T::GetTypeIDStatic());
			if (it != m_componentsByType.end())
				return &it->second;

			return nullptr;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		bool HasComponent() const
		{
			return GetComponent<T>() != nullptr;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
		bool RemoveComponent()
		{
			if constexpr (std::is_same_v<T, Transform>)
				return false;

			auto it = m_componentsByType.find(T::GetTypeIDStatic());
			if (it == m_componentsByType.end() || it->second.empty())
				return false;

			Component* component = it->second.back();
			return RemoveComponent(component);
		}

		bool RemoveComponent(Component* component);

		// If a child is set up, then it means this is its parent and will update it accordingly
		void AddChild(const std::shared_ptr<Entity>& child, bool setOrder = true);
		void RemoveChild(const std::shared_ptr<Entity>& child, bool setOrder = true);
		void RemoveChild(UUID childUuid, bool setOrder = true);
		void ReorderChild(const std::shared_ptr<Entity>& child, int newIndex);

		const UUID& GetUUID() const;
		const std::string& GetName() const;
		bool GetIsActive() const;
		bool GetIsActiveInHierarchy() const;
		bool GetIsStatic() const;
		bool GetIsStaticInHierarchy() const;
		std::shared_ptr<Entity> GetChild(UUID uuid, bool deepSearch = false) const;
		std::shared_ptr<Entity> GetChild(int index) const;
		std::shared_ptr<Entity> GetChild(const std::string& name, bool deepSearch = false) const;

		int GetComponentCount() const;
		int GetOrder() const;

		int GetChildCount() const;
		const std::vector<std::shared_ptr<Entity>>& GetChildren() const;
		std::weak_ptr<Entity> GetParent() const;
		std::vector<Component*> GetComponents() const;
		const std::vector<std::unique_ptr<Component>>& GetComponentsRaw() const { return m_components; }
		Transform* GetTransform() const;

		void SetUUID(const std::string& uuid);
		void SetUUID(UUID uuid);
		void SetName(const std::string& name);
		void SetIsActive(bool active);
		void SetIsStatic(bool active);

		void SetOrder(int order);
		void SortChildrenByOrder();

		// If a parent is set up, then it means this is its child and will update it accordingly
		void SetParent(const std::shared_ptr<Entity>& parent, bool keepLocal = true, bool setOrder = true);


		void GetRecursiveChildren(std::vector<std::shared_ptr<Entity>>& childrenEntities);

	private:

	private:
		std::weak_ptr<Entity> m_parentEntity;
		std::vector<std::shared_ptr<Entity>> m_childrenEntities;
		std::vector<std::unique_ptr<Component>> m_components; // Might want to re-do this to a map for optimization
		std::unordered_map<UUID, Component*> m_componentsByUUID;

		std::unordered_map<size_t, std::vector<Component*>> m_componentsByType;

		Transform* m_transform = nullptr;

		UUID m_uuid;
		std::string m_name;
		bool m_isActive = true;
		bool m_isStatic = true;
		int m_order = -1;
	};
}