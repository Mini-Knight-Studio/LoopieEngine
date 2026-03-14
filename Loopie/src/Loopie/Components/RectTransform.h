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

		// Layout
		const vec2& GetAnchorMin() const { return m_anchorMin; }
		const vec2& GetAnchorMax() const { return m_anchorMax; }
		const vec2& GetPivot() const { return m_pivot; }
		const vec2& GetSizeDelta() const { return m_sizeDelta; }

		void SetAnchorMin(const vec2& v);
		void SetAnchorMax(const vec2& v);
		void SetPivot(const vec2& v);
		void SetSizeDelta(const vec2& v);

		vec2 GetSize() const override;
		float GetWidth() const override;
		float GetHeight() const override;

		void SetWidth(float w) override;
		void SetHeight(float h) override;

		vec3 GetLocalBoundsMin() const override { return { 0.0f, 0.0f, 0.0f }; }
		vec3 GetLocalBoundsMax() const override { return { GetWidth(), GetHeight(), 0.0f }; }

		vec2 GetRectMinCanvasSpace(const vec2& parentSize) const;
		vec2 GetRectSizeCanvasSpace(const vec2& parentSize) const;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		static vec2 Clamp01(const vec2& v);

	private:
		vec2 m_anchorMin = vec2(0.5f, 0.5f);
		vec2 m_anchorMax = vec2(0.5f, 0.5f);
		vec2 m_pivot = vec2(0.5f, 0.5f);
		vec2 m_sizeDelta = vec2(50.0f, 50.0f);

		bool m_inheritedParentSize = false;
	};
}