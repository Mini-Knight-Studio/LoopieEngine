#include "RectTransform.h"

namespace Loopie
{
	RectTransform::RectTransform(float w, float h) : Transform()
	{
		m_sizeDelta = vec2(w, h);
	}

	void RectTransform::Init()
	{
		Transform::Init();

		if (m_inheritedParentSize)
			return;

		const auto parent = GetOwner()->GetParent().lock();
		if (!parent)
			return;

		const auto parentTransform = parent->GetTransform();
		if (!parentTransform->IsRectTransform())
			return;

		m_sizeDelta = vec2(parentTransform->GetWidth(), parentTransform->GetHeight());
		m_anchorMin = vec2(0.0f, 0.0f);
		m_anchorMax = vec2(1.0f, 1.0f);
		m_pivot = vec2(0.5f, 0.5f);

		m_inheritedParentSize = true;
		MarkLocalDirty();
	}

	vec2 RectTransform::Clamp01(const vec2& v)
	{
		return vec2(glm::clamp(v.x, 0.0f, 1.0f), glm::clamp(v.y, 0.0f, 1.0f));
	}

	void RectTransform::SetAnchorMin(const vec2& v)
	{
		m_anchorMin = Clamp01(v);
		MarkWorldDirty();
	}

	void RectTransform::SetAnchorMax(const vec2& v)
	{
		m_anchorMax = Clamp01(v);
		MarkWorldDirty();
	}

	void RectTransform::SetPivot(const vec2& v)
	{
		m_pivot = Clamp01(v);
		MarkWorldDirty();
	}

	void RectTransform::SetSizeDelta(const vec2& v)
	{
		m_sizeDelta = v;
		MarkWorldDirty();
	}

	vec2 RectTransform::GetSize() const
	{
		return m_sizeDelta;
	}

	float RectTransform::GetWidth() const
	{
		return m_sizeDelta.x;
	}

	float RectTransform::GetHeight() const
	{
		return m_sizeDelta.y;
	}

	void RectTransform::SetWidth(float w)
	{
		m_sizeDelta.x = w;
		MarkWorldDirty();
	}

	void RectTransform::SetHeight(float h)
	{
		m_sizeDelta.y = h;
		MarkWorldDirty();
	}

	vec2 RectTransform::GetRectSizeCanvasSpace(const vec2& parentSize) const
	{
		const vec2 anchorSpan = (m_anchorMax - m_anchorMin);
		const vec2 stretched = vec2(anchorSpan.x * parentSize.x, anchorSpan.y * parentSize.y);
		return stretched + m_sizeDelta;
	}

	vec2 RectTransform::GetRectMinCanvasSpace(const vec2& parentSize) const
	{
		const vec2 size = GetRectSizeCanvasSpace(parentSize);
		const vec2 anchorRef = vec2(m_anchorMin.x * parentSize.x, m_anchorMin.y * parentSize.y);

		const vec2 anchoredPosition(m_localPosition.x, m_localPosition.y);

		return anchorRef + anchoredPosition - (m_pivot * size);
	}

	JsonNode RectTransform::Serialize(JsonNode& parent) const
	{
		JsonNode rectTransformObj = parent.CreateObjectField("recttransform");

		JsonNode node = rectTransformObj.CreateObjectField("position");
		node.CreateField("x", m_localPosition.x);
		node.CreateField("y", m_localPosition.y);
		node.CreateField("z", m_localPosition.z);

		node = rectTransformObj.CreateObjectField("rotation");
		node.CreateField("x", m_localRotation.x);
		node.CreateField("y", m_localRotation.y);
		node.CreateField("z", m_localRotation.z);
		node.CreateField("w", m_localRotation.w);

		node = rectTransformObj.CreateObjectField("scale");
		node.CreateField("x", m_localScale.x);
		node.CreateField("y", m_localScale.y);
		node.CreateField("z", m_localScale.z);

		vec3 localEulerAngles = GetLocalEulerAngles();
		node = rectTransformObj.CreateObjectField("euler_angles");
		node.CreateField("x", localEulerAngles.x);
		node.CreateField("y", localEulerAngles.y);
		node.CreateField("z", localEulerAngles.z);

		JsonNode layout = rectTransformObj.CreateObjectField("layout");

		node = layout.CreateObjectField("anchor_min");
		node.CreateField("x", m_anchorMin.x);
		node.CreateField("y", m_anchorMin.y);

		node = layout.CreateObjectField("anchor_max");
		node.CreateField("x", m_anchorMax.x);
		node.CreateField("y", m_anchorMax.y);

		node = layout.CreateObjectField("pivot");
		node.CreateField("x", m_pivot.x);
		node.CreateField("y", m_pivot.y);

		node = layout.CreateObjectField("size_delta");
		node.CreateField("x", m_sizeDelta.x);
		node.CreateField("y", m_sizeDelta.y);

		return rectTransformObj;
	}

	void RectTransform::Deserialize(const JsonNode& data)
	{
		Transform::Deserialize(data);

		if (data.Contains("layout"))
		{
			const JsonNode layout = data.Child("layout");

			const JsonNode aMin = layout.Child("anchor_min");
			if (aMin.IsValid() && aMin.IsObject())
				m_anchorMin = vec2(aMin.GetValue<float>("x", m_anchorMin.x).Result, aMin.GetValue<float>("y", m_anchorMin.y).Result);

			const JsonNode aMax = layout.Child("anchor_max");
			if (aMax.IsValid() && aMax.IsObject())
				m_anchorMax = vec2(aMax.GetValue<float>("x", m_anchorMax.x).Result, aMax.GetValue<float>("y", m_anchorMax.y).Result);

			const JsonNode piv = layout.Child("pivot");
			if (piv.IsValid() && piv.IsObject())
				m_pivot = vec2(piv.GetValue<float>("x", m_pivot.x).Result, piv.GetValue<float>("y", m_pivot.y).Result);

			const JsonNode sd = layout.Child("size_delta");
			if (sd.IsValid() && sd.IsObject())
				m_sizeDelta = vec2(sd.GetValue<float>("x", m_sizeDelta.x).Result, sd.GetValue<float>("y", m_sizeDelta.y).Result);

			m_anchorMin = Clamp01(m_anchorMin);
			m_anchorMax = Clamp01(m_anchorMax);
			m_pivot = Clamp01(m_pivot);
		}
		else
		{
			JsonNode node = data.Child("size");
			if (node.IsValid() && node.IsObject())
			{
				m_sizeDelta.x = node.GetValue<float>("width", m_sizeDelta.x).Result;
				m_sizeDelta.y = node.GetValue<float>("height", m_sizeDelta.y).Result;
			}
		}
	}
}
