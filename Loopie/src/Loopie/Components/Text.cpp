#include "Text.h"

#include "Loopie/Components/RectTransform.h"
#include "Loopie/Importers/FontImporter.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Render/Gizmo.h"

namespace Loopie {

	Text::~Text()
	{
		if (m_font)
			m_font->DecrementReferenceCount();
	}

	void Text::Init()
	{
		if (!GetOwner()->HasComponent<RectTransform>())
			GetOwner()->ReplaceTransform<RectTransform>();
	}

	void Text::RenderGizmo()
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

	void Text::SetFont(const std::shared_ptr<Font>& font)
	{
		if (m_font == font)
			return;

		if (m_font)
			m_font->DecrementReferenceCount();

		m_font = font;

		if (m_font)
			m_font->IncrementReferenceCount();
	}

	vec2 Text::MeasureLocalSizeFixed() const
	{
		if (!m_font || m_font->GetRendererId() == 0 || m_text.empty())
			return vec2(0.0f);

		float x = 0.0f;
		float y = 0.0f;

		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;

		const float baseScale = (m_scale <= 0.0f) ? 1.0f : m_scale;

		const float px = (m_fontSize <= 0.0f) ? (float)m_font->GetPixelSize() : m_fontSize;
		const float denom = (float)std::max(1, m_font->GetPixelSize());
		const float fontScale = (px / denom) * baseScale;

		for (size_t i = 0; i < m_text.size(); i++)
		{
			const unsigned char ch = (unsigned char)m_text[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= (float)m_font->GetLineHeight() * fontScale;
				continue;
			}

			const FontGlyph* g = m_font->GetGlyph((int)ch);
			if (!g)
				continue;

			const float xpos = x + (float)g->bearing.x * fontScale;
			const float ypos = y - ((float)g->size.y - (float)g->bearing.y) * fontScale;
			const float w = (float)g->size.x * fontScale;
			const float h = (float)g->size.y * fontScale;

			minX = std::min(minX, xpos);
			minY = std::min(minY, ypos);
			maxX = std::max(maxX, xpos + w);
			maxY = std::max(maxY, ypos + h);

			x += ((float)g->advance / 64.0f) * fontScale;
		}

		const float textW = std::max(0.0f, maxX - minX);
		const float textH = std::max(0.0f, maxY - minY);

		return vec2(textW, textH);
	}

	JsonNode Text::Serialize(JsonNode& parent) const
	{
		JsonNode textNode = parent.CreateObjectField("text");

		textNode.CreateField<std::string>("value", m_text);

		JsonNode colorObj = textNode.CreateObjectField("color");
		colorObj.CreateField<float>("r", m_color.r);
		colorObj.CreateField<float>("g", m_color.g);
		colorObj.CreateField<float>("b", m_color.b);
		colorObj.CreateField<float>("a", m_color.a);

		textNode.CreateField<float>("scale", m_scale);

		textNode.CreateField<int>("size_mode", (int)m_sizeMode);
		textNode.CreateField<float>("font_size", m_fontSize);
		textNode.CreateField<int>("horizontal_alignment", (int)m_horizontalAlignment);
		textNode.CreateField<int>("vertical_alignment", (int)m_verticalAlignment);

		if (m_font)
			textNode.CreateField<std::string>("font_uuid", m_font->GetUUID().Get());

		return textNode;
	}

	void Text::Deserialize(const JsonNode& data)
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

		m_sizeMode = (TextSizeMode)data.GetValue<int>("size_mode", (int)TextSizeMode::AutoSize).Result;
		m_fontSize = data.GetValue<float>("font_size", 24.0f).Result;
		m_horizontalAlignment = (TextHorizontalAlignment)data.GetValue<int>("horizontal_alignment", (int)TextHorizontalAlignment::Left).Result;
		m_verticalAlignment = (TextVerticalAlignment)data.GetValue<int>("vertical_alignment", (int)TextVerticalAlignment::Top).Result;

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
}
