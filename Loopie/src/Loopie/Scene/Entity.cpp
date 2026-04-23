#include "Entity.h"

#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/Log.h"

#include <algorithm>

namespace Loopie {
	Entity::Entity(const std::string& name) : m_name(name)
	{
		m_components.reserve(5);
		m_childrenEntities.reserve(10);
	}

	Entity::~Entity()
	{
		m_components.clear();
		m_childrenEntities.clear();
	}

	bool Entity::RemoveComponent(Component* component)
	{
		//if (component->GetTypeID() == m_transform->GetTypeID())
			//return false;

		for (size_t i = 0; i < m_components.size(); i++)
		{
			if (m_components[i].get() == component) {
				if (component == m_transform)
					m_transform = nullptr;
				m_components.erase(m_components.begin() + i);
				return true;
			}
		}
		m_transform->MarkHasChangedThisFrame();
		return false;
	}

	void Entity::AddChild(const std::shared_ptr<Entity>& child, bool setOrder)
	{
		if (child && child.get() != this)
		{
			std::shared_ptr<Entity> childParent = child->m_parentEntity.lock();
			if (childParent)
			{
				childParent->RemoveChild(child);
			}

			if (setOrder) {
				child->m_order = (int)m_childrenEntities.size();
				m_childrenEntities.push_back(child);
				child->m_parentEntity = weak_from_this();
			}	
		}
	}

	void Entity::RemoveChild(const std::shared_ptr<Entity>& child, bool setOrder)
	{
		auto it = std::find(m_childrenEntities.begin(), m_childrenEntities.end(), child);
		if (it != m_childrenEntities.end())
		{
			(*it)->m_parentEntity.reset();
			(*it)->m_order = 0;
			m_childrenEntities.erase(it);

			if (setOrder) {
				for (int i = 0; i < (int)m_childrenEntities.size(); i++)
					m_childrenEntities[i]->m_order = i;
			}
		}
	}

	void Entity::RemoveChild(UUID childUuid, bool setOrder)
	{
		for (auto it = m_childrenEntities.begin(); it != m_childrenEntities.end(); ++it)
		{
			if ((*it)->GetUUID() == childUuid)
			{
				(*it)->m_parentEntity.reset();
				(*it)->m_order = 0;
				m_childrenEntities.erase(it);

				if (setOrder) {
					for (int i = 0; i < (int)m_childrenEntities.size(); i++)
						m_childrenEntities[i]->m_order = i;
				}
				return;
			}
		}
	}

	void Entity::ReorderChild(const std::shared_ptr<Entity>& child, int newIndex)
	{
		auto it = std::find(m_childrenEntities.begin(), m_childrenEntities.end(), child);
		if (it == m_childrenEntities.end())
			return;

		newIndex = std::clamp(newIndex, 0, (int)m_childrenEntities.size() - 1);
		int currentIndex = (int)(it - m_childrenEntities.begin());

		if (currentIndex == newIndex)
			return;

		m_childrenEntities.erase(it);
		m_childrenEntities.insert(m_childrenEntities.begin() + newIndex, child);

		for (int i = 0; i < (int)m_childrenEntities.size(); i++)
			m_childrenEntities[i]->m_order = i;
	}

	const UUID& Entity::GetUUID() const
	{
		return m_uuid;
	}

	const std::string& Entity::GetName() const
	{
		return m_name;
	}

	bool Entity::GetIsActive() const
	{
		std::shared_ptr<Entity> parent = GetParent().lock();

		if (parent) {
			return m_isActive && parent->GetIsActive();
		}

		return m_isActive;
	}

	bool Entity::GetIsActiveInHierarchy() const
	{
		return m_isActive;
	}

	bool Entity::GetIsStatic() const
	{
		std::shared_ptr<Entity> parent = GetParent().lock();

		if (parent) {
			return m_isStatic && parent->GetIsStatic();
		}

		return m_isStatic;
	}

	bool Entity::GetIsStaticInHierarchy() const
	{
		return m_isStatic;
	}

	std::shared_ptr<Entity> Entity::GetChild(UUID uuid) const
	{
		for (const auto& child : m_childrenEntities)
		{
			if (child->GetUUID() == uuid)
			{
				return child;
			}
		}
		return nullptr;
	}

	std::shared_ptr<Entity> Entity::GetChild(int index) const
	{
		if (index >= 0 && index < GetChildCount())
			return m_childrenEntities[index];
		return nullptr;
	}

	std::shared_ptr<Entity> Entity::GetChild(const std::string& name, bool deepSearch) const
	{
		for (const auto& child : m_childrenEntities)
		{
			if (child->GetName() == name)
				return child;
		}
		if (deepSearch) {
			for (const auto& child : m_childrenEntities) {
				auto result = child->GetChild(name, true);
				if (result)
					return result;
			}
		}
		return nullptr;
	}

	int Entity::GetComponentCount() const
	{
		return m_components.size();
	}

	int Entity::GetChildCount() const
	{
		return m_childrenEntities.size();
	}

	int Entity::GetOrder() const
	{
		return m_order;
	}

	const std::vector<std::shared_ptr<Entity>>& Entity::GetChildren() const
	{
		return m_childrenEntities;
	}

	std::weak_ptr<Entity> Entity::GetParent() const
	{
		return m_parentEntity;
	}

	std::vector<Component*> Entity::GetComponents() const
	{
		std::vector<Component*> outComponents;
		outComponents.reserve(m_components.size());

		for (const auto& component : m_components) {
			outComponents.push_back(component.get());
		}

		return outComponents;
	}

	Component* Entity::GetComponent(UUID uuid) {
		auto it = m_componentsByUUID.find(uuid);
		if (it != m_componentsByUUID.end())
			return it->second;
		return nullptr;
	}

	void Entity::OnComponentUUIDChange(Component* component, UUID oldUUID) {
		auto it = m_componentsByUUID.find(oldUUID);
		if (it != m_componentsByUUID.end()) {
			m_componentsByUUID.erase(it);
			m_componentsByUUID[component->GetUUID()] = component;
		}
	}

	Transform* Entity::GetTransform() const
	{
		return m_transform;
	}

	void Entity::SetUUID(const std::string& uuid)
	{
		m_uuid = UUID(uuid);
	}

	void Entity::SetUUID(UUID uuid)
	{
		m_uuid = uuid;
	}

	void Entity::SetName(const std::string& name)
	{
		m_name = name;
	}

	void Entity::SetIsActive(bool active)
	{
		m_isActive = active;
	}

	void Entity::SetIsStatic(bool active)
	{
		m_isStatic = active;
	}

	void Entity::SetOrder(int order)
	{
		m_order = order;
	}

	void Entity::SortChildrenByOrder()
	{
		if (m_childrenEntities.empty())
			return;

		std::stable_sort(m_childrenEntities.begin(), m_childrenEntities.end(),
			[](const std::shared_ptr<Entity>& a, const std::shared_ptr<Entity>& b)
			{
				if (a->m_order < 0 && b->m_order < 0) return false;
				if (a->m_order < 0) return false;
				if (b->m_order < 0) return true;
				return a->m_order < b->m_order;
			});

		for (int i = 0; i < (int)m_childrenEntities.size(); i++)
			m_childrenEntities[i]->m_order = i;
	}

	void Entity::SetParent(const std::shared_ptr<Entity>& parent, bool keepLocal, bool setOrder)
	{
		// Prevents parenting to its own son
		if (parent == shared_from_this())
		{
			Log::Warn("Cannot parent entity to itself.");
			return;
		}

		if (parent)
		{
			std::vector<std::shared_ptr<Entity>> allChildren;
			GetRecursiveChildren(allChildren);

			for (const auto& child : allChildren)
			{
				if (parent == child)
				{
					Log::Warn("Cannot parent entity to one of its descendants, as it would create an infinite loop.");
					return;
				}
			}
		}

		Transform* transform = GetTransform();
		matrix4 worldMatrix;
		if (!keepLocal && transform)
		{
			worldMatrix = transform->GetLocalToWorldMatrix();
		}

		std::shared_ptr<Entity> currentParent = m_parentEntity.lock();
		if (currentParent)
		{
			currentParent->RemoveChild(shared_from_this(), setOrder);
		}

		m_parentEntity = parent;

		if (parent && (parent != shared_from_this()))
		{
			parent->AddChild(shared_from_this(), setOrder);
		}

		if (!keepLocal && transform)
		{
			transform->SetWorldMatrix(worldMatrix);
		}
		else if (transform)
		{
			transform->MarkWorldDirty();
		}
	}

	void Entity::GetRecursiveChildren(std::vector<std::shared_ptr<Entity>>& childrenEntities)
	{
		for (const auto& child : m_childrenEntities)
		{
			childrenEntities.push_back(child);
			child->GetRecursiveChildren(childrenEntities);
		}
	}
}