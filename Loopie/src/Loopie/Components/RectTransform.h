#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"

namespace Loopie
{
	class RectTransform : public Transform
	{
	public:
		DEFINE_TYPE(RectTransform)

        RectTransform(float w = 50, float h = 50);

		void Init() override;

		bool IsRectTransform() const override { return true; }
        bool HasSize() const override { return true; }

        vec2 GetSize() const override { return { m_width, m_height }; }

        float GetWidth() const override;
        float GetHeight() const override;

        void SetWidth(float w) override;
		void SetHeight(float h) override;

        vec3 RectTransform::GetLocalBoundsMin() const override { return { 0.0f, 0.0f, 0.0f }; }
        vec3 RectTransform::GetLocalBoundsMax() const override { return { m_width,  m_height, 0.0f }; }

        JsonNode Serialize(JsonNode& parent) const override;
        void Deserialize(const JsonNode& data) override;

    private:
        float m_width;
        float m_height;
        bool m_inheritedParentSize = false;
	};
}