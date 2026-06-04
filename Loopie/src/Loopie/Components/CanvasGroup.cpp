#include "CanvasGroup.h"

namespace Loopie
{
	JsonNode CanvasGroup::Serialize(JsonNode& parent) const
	{
		JsonNode obj = parent.CreateObjectField("canvas_group");
		obj.CreateField<float>("alpha", m_alpha);
		return obj;
	}

	void CanvasGroup::Deserialize(const JsonNode& data)
	{
		m_alpha = glm::clamp(data.GetValue<float>("alpha", m_alpha).Result, 0.0f, 1.0f);
	}

	void CanvasGroup::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		(void)entity;
		const CanvasGroup& otherGroup = static_cast<const CanvasGroup&>(other);
		m_alpha = otherGroup.m_alpha;
	}
}