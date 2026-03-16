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

        vec2 GetSize() const override { return { m_sizeDelta.x, m_sizeDelta.y }; }

        float GetWidth() const override;
        float GetHeight() const override;

        void SetWidth(float w) override;
		void SetHeight(float h) override;

		vec2 GetAnchorMin() const { return m_anchorMin; }
		vec2 GetAnchorMax() const { return m_anchorMax; }
		vec2 GetPivot() const { return m_pivot; }
		vec2 GetAnchoredPosition() const { return vec2(m_localPosition.x, m_localPosition.y); }

		void SetAnchorMin(vec2& anchorMin);
		void SetAnchorMax(vec2& anchorMax);
		void SetPivot(vec2& pivot);
		void SetAnchoredPosition(const vec2& anchoredPosition);

        vec3 RectTransform::GetLocalBoundsMin() const override;
        vec3 RectTransform::GetLocalBoundsMax() const override;

        void RefreshMatrices() const override;

        JsonNode Serialize(JsonNode& parent) const override;
        void Deserialize(const JsonNode& data) override;

    private:
		vec2 GetParentSize() const;
		vec2 GetComputedSize() const;
		vec2 GetAnchorReferencePoint(const vec2& parentSize) const;
		vec2 ComputePivotTranslation(const vec2& size) const;

		vec2 m_anchorMin = vec2(0.0f);
		vec2 m_anchorMax = vec2(0.0f);
		vec2 m_pivot = vec2(0.0f, 0.0f);
		vec2 m_sizeDelta = vec2(0.0f);

        bool m_inheritedParentSize = false;
	};
}