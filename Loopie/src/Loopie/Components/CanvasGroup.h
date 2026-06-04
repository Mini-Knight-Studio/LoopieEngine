#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"

#include <memory>

namespace Loopie
{
	class Entity;

	class CanvasGroup : public Component
	{
	public:
		DEFINE_TYPE(CanvasGroup)

		CanvasGroup() = default;
		~CanvasGroup() = default;

		void Init() {}

		JsonNode Serialize(JsonNode& parent) const;
		void Deserialize(const JsonNode& data);
		void Clone(const std::shared_ptr<Entity> entity, const Component& other);

		float GetAlpha() const { return m_alpha; }
		void SetAlpha(float alpha) { m_alpha = glm::clamp(alpha, 0.0f, 1.0f); }

	private:
		float m_alpha = 1.0f;
	};
}