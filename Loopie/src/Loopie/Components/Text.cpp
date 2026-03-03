#include "Text.h"

#include "Loopie/Components/RectTransform.h"
#include "Loopie/Importers/FontImporter.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Render/Gizmo.h"

void Loopie::Text::Init()
{
	if (!GetOwner()->HasComponent<RectTransform>())
		GetOwner()->ReplaceTransform<RectTransform>();
}

void Loopie::Text::RenderGizmo()
{
	auto* rt = GetOwner() ? GetOwner()->GetComponent<RectTransform>() : nullptr;
	if (!rt)
		return;

	const matrix4& m = rt->GetLocalToWorldMatrix();
	const vec3 localMin = rt->GetLocalBoundsMin();
	const vec3 localMax = rt->GetLocalBoundsMax();

	const vec3 corners[8] =
	{
		{ localMin.x, localMin.y, 0.0f },
		{ localMax.x, localMin.y, 0.0f },
		{ localMin.x, localMax.y, 0.0f },
		{ localMax.x, localMax.y, 0.0f },
		{ localMin.x, localMin.y, 0.0f },
		{ localMax.x, localMin.y, 0.0f },
		{ localMin.x, localMax.y, 0.0f },
		{ localMax.x, localMax.y, 0.0f },
	};

	AABB aabb;
	aabb.SetNegativeInfinity();
	for (int i = 0; i < 4; ++i)
	{
		const vec3 w = vec3(m * vec4(corners[i], 1.0f));
		aabb.Enclose(w);
	}

	Gizmo::DrawCube(aabb, Color::BLUE);
}

void Loopie::Text::SetFont(const std::shared_ptr<Font>& font)
{
	if (m_font == font)
		return;

	if (m_font)
		m_font->DecrementReferenceCount();

	m_font = font;

	if (m_font)
		m_font->IncrementReferenceCount();
}

Loopie::JsonNode Loopie::Text::Serialize(JsonNode& parent) const
{
	JsonNode textNode = parent.CreateObjectField("text");

	textNode.CreateField<std::string>("value", m_text);

	JsonNode colorObj = textNode.CreateObjectField("color");
	colorObj.CreateField<float>("r", m_color.r);
	colorObj.CreateField<float>("g", m_color.g);
	colorObj.CreateField<float>("b", m_color.b);
	colorObj.CreateField<float>("a", m_color.a);

	textNode.CreateField<float>("scale", m_scale);

	if (m_font)
		textNode.CreateField<std::string>("font_uuid", m_font->GetUUID().Get());

	return textNode;
}

void Loopie::Text::Deserialize(const JsonNode& data)
{
	m_text = data.GetValue<std::string>("value", "Text").Result;

	JsonNode colorObj = data.Child("color");
	if (colorObj.IsValid() && colorObj.IsObject())
	{
		m_color.r = colorObj.GetValue<float>("r", 1.0f).Result;
		m_color.g = colorObj.GetValue<float>("g", 1.0f).Result;
		m_color.b = colorObj.GetValue<float>("b", 1.0f).Result;
		m_color.a = colorObj.GetValue<float>("a", 1.0f).Result;
	}
	else
	{
		m_color = vec4(1.0f);
	}

	m_scale = data.GetValue<float>("scale", 1.0f).Result;

	if (data.Contains("font_uuid"))
	{
		UUID uuid = data.GetValue<std::string>("font_uuid").Result;
		Metadata* meta = AssetRegistry::GetMetadata(uuid);
		if (meta && meta->Type == ResourceType::FONT)
		{
			auto font = ResourceManager::GetFont(*meta);
			if (font && font->Load())
			{
				SetFont(font);
				return;
			}
		}
	}

	SetFont(nullptr);
}
