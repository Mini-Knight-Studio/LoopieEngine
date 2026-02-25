#include "Component.h"


#include "Loopie/Scene/Entity.h"
#include "Loopie/Components/Transform.h"

namespace Loopie {
	Component::~Component(){}
	Transform* Component::GetTransform() const
	{
		return GetOwner()->GetTransform();
	}

	const UUID& Component::GetUUID() const
	{
		return m_uuid;
	}

	bool Component::GetIsActive() const
	{
		return m_isActive && m_owner.lock()->GetIsActive();
	}

	void Component::SetIsActive(bool active)
	{
		m_isActive = active;
	}

	void Component::SetUUID(const std::string uuid)
	{
		UUID old = m_uuid;
		m_uuid = UUID(uuid);
		m_owner.lock()->OnComponentUUIDChange(this, old);
	}

	JsonNode Component::Serialize(JsonNode& parent) const
	{
		return JsonNode();
	}

	void Component::Deserialize(const JsonNode& data)
	{
		return;
	}
}