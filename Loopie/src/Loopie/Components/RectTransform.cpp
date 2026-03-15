#include "RectTransform.h"

static Loopie::vec2 Clamp01(const Loopie::vec2& v)
{
	return Loopie::vec2(glm::clamp(v.x, 0.0f, 1.0f), glm::clamp(v.y, 0.0f, 1.0f));
}

Loopie::RectTransform::RectTransform(float w, float h) : Transform()
{
	m_sizeDelta = vec2(w, h);
}

void Loopie::RectTransform::Init()
{
    Transform::Init();
}

void Loopie::RectTransform::SetAnchorMin(vec2& anchorMin)
{
	m_anchorMin = Clamp01(anchorMin);
	MarkWorldDirty();
}

void Loopie::RectTransform::SetAnchorMax(vec2& anchorMax)
{
	m_anchorMax = Clamp01(anchorMax);
	MarkWorldDirty();
}

void Loopie::RectTransform::SetPivot(vec2& pivot)
{
	m_pivot = Clamp01(pivot);
	MarkWorldDirty();
}

void Loopie::RectTransform::SetAnchoredPosition(const vec2& anchoredPosition)
{
	m_localPosition.x = anchoredPosition.x;
	m_localPosition.y = anchoredPosition.y;
	MarkLocalDirty();
}

Loopie::vec2 Loopie::RectTransform::GetParentSize() const
{
	const auto parent = GetOwner()->GetParent().lock();
	if (!parent)
		return vec2(0.0f);

	const auto parentTransform = parent->GetTransform();
	if (!parentTransform || !parentTransform->HasSize())
		return vec2(0.0f);

	return parentTransform->GetSize();
}

Loopie::vec2 Loopie::RectTransform::GetComputedSize() const
{
	const vec2 parentSize = GetParentSize();
	const vec2 anchorRange = m_anchorMax - m_anchorMin;
	const vec2 stretchSize = parentSize * anchorRange;
	return stretchSize + m_sizeDelta;
}

Loopie::vec2 Loopie::RectTransform::GetAnchorReferencePoint(const vec2& parentSize) const
{
	const vec2 anchorCenter = (m_anchorMin + m_anchorMax) * 0.5f;
	return parentSize * anchorCenter;
}

Loopie::vec2 Loopie::RectTransform::ComputePivotTranslation(const vec2& size) const
{
	return -(m_pivot * size);
}

float Loopie::RectTransform::GetWidth() const
{
	return GetComputedSize().x;
}

float Loopie::RectTransform::GetHeight() const
{
	return GetComputedSize().y;
}

Loopie::vec3 Loopie::RectTransform::GetLocalBoundsMin() const
{
	const vec2 size = GetComputedSize();
	const vec2 min = ComputePivotTranslation(size);
	return vec3(min.x, min.y, 0.0f);
}

Loopie::vec3 Loopie::RectTransform::GetLocalBoundsMax() const
{
	const vec2 size = GetComputedSize();
	const vec2 min = ComputePivotTranslation(size);
	const vec2 max = min + size;
	return vec3(max.x, max.y, 0.0f);
}

void Loopie::RectTransform::RefreshMatrices() const
{
	if (!IsDirty()) return;

	const vec2 parentSize = GetParentSize();
	const vec2 computedSize = GetComputedSize();
	const vec2 anchorReference = GetAnchorReferencePoint(parentSize);
	const vec2 pivotTranslation = ComputePivotTranslation(computedSize);

	vec3 effectiveLocalPosition = m_localPosition;
	effectiveLocalPosition.x += anchorReference.x + pivotTranslation.x;
	effectiveLocalPosition.y += anchorReference.y + pivotTranslation.y;

	m_localMatrix = translate(matrix4(1.0f), effectiveLocalPosition) * toMat4(m_localRotation) * scale(matrix4(1.0f), m_localScale);

	if (auto parent = GetOwner()->GetParent().lock())
	{
		Transform* transform = parent->GetTransform();
		transform->GetLocalToWorldMatrix();
		m_localToWorld = transform->GetLocalToWorldMatrix() * m_localMatrix;
	}
	else
	{
		m_localToWorld = m_localMatrix;
	}

	m_worldToLocal = inverse(m_localToWorld);

	m_localDirty = false;
	m_worldDirty = false;

	m_transformNotifier.Notify(TransformNotification::OnChanged);
}

void Loopie::RectTransform::SetWidth(float w)
{
	m_sizeDelta.x = w;
    MarkWorldDirty();
}

void Loopie::RectTransform::SetHeight(float h)
{
	m_sizeDelta.y = h;
    MarkWorldDirty();
}

Loopie::JsonNode Loopie::RectTransform::Serialize(JsonNode& parent) const
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

void Loopie::RectTransform::Deserialize(const JsonNode& data)
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
