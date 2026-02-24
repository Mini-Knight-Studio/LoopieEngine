#include "RectTransform.h"

Loopie::RectTransform::RectTransform(float w, float h) : Transform()
{
	m_width = w;
	m_height = h;
}

float Loopie::RectTransform::GetWidth() const
{
	return m_width;
}

float Loopie::RectTransform::GetHeight() const
{
	return m_height;
}

void Loopie::RectTransform::SetWidth(float w)
{
	m_width = w;
}

void Loopie::RectTransform::SetHeight(float h)
{
	m_height = h;
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

	JsonNode rec = rectTransformObj.CreateObjectField("size");
	rec.CreateField("width", m_width);
	rec.CreateField("height", m_height);

    return rectTransformObj;
}

void Loopie::RectTransform::Deserialize(const JsonNode& data)
{
	Transform::Deserialize(data);
	JsonNode node = data.Child("size");
	if (node.IsValid() && node.IsObject())
	{
		m_width = node.GetValue<float>("width", 100.0f).Result;
		m_height = node.GetValue<float>("height", 100.0f).Result;
	}
}
