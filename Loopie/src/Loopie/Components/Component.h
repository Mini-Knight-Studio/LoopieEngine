#pragma once
#include <memory>

#include "Loopie/Core/IIdentificable.h"
#include "Loopie/Core/ISerializable.h"
#include "Loopie/Core/UUID.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Files/Json.h"

//#include <nlohmann/json.hpp>

namespace Loopie {
	class Entity;
	class Transform;
	class UIElement;
	class UINavigable;

	class Component : public IIdentificable, public ISerializable
	{
		friend class Entity;
	public:
		Component() = default;
		virtual ~Component();

		// Getters
		Transform* GetTransform() const;
		std::shared_ptr<Entity> GetOwner() const { return m_owner.lock(); }
		const UUID& GetUUID() const;
		bool GetIsActive() const;
		bool GetLocalIsActive() const;

		// Default Calls
		virtual void OnUpdate() {};
		virtual void RenderGizmo() const {};

		// Optional UI queries (avoids RTTI/dynamic_cast across modules)
		virtual UIElement* AsUIElement() { return nullptr; }
		virtual const UIElement* AsUIElement() const { return nullptr; }
		virtual UINavigable* AsUINavigable() { return nullptr; }
		virtual const UINavigable* AsUINavigable() const { return nullptr; }

		// Setters
		void SetIsActive(bool active);
		void SetUUID(const std::string uuid);

		// Serialize & Deserialize
		virtual JsonNode Serialize(JsonNode& parent) const = 0;
		virtual void Deserialize(const JsonNode& data) = 0;
		virtual void OnSceneDeserialized() {};

		virtual void Init() = 0;

	private:
		std::weak_ptr<Entity> m_owner;
		UUID m_uuid;
		bool m_isActive = true;
	};
}