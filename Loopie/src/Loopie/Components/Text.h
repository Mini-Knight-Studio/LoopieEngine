#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Resources/Types/Font.h"

#include <memory>
#include <string>

namespace Loopie
{
	enum class TextHorizontalAlignment
	{
		Left,
		Center,
		Right
	};

	enum class TextVerticalAlignment
	{
		Top,
		Middle,
		Bottom
	};

	enum class TextSizeMode
	{
		AutoSize,
		FixedSize
	};

	class Text : public Component
	{
	public:
		DEFINE_TYPE(Text)

		Text() = default;
		~Text();

		void Init() override;

		void RenderGizmo() override;

		const std::string& GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; }

		const vec4& GetColor() const { return m_color; }
		void SetColor(const vec4& color) { m_color = color; }

		float GetScale() const { return m_scale; }
		void SetScale(float scale) { m_scale = scale; }

		TextSizeMode GetSizeMode() const { return m_sizeMode; }
		void SetSizeMode(TextSizeMode mode) { m_sizeMode = mode; }

		float GetFontSize() const { return m_fontSize; }
		void SetFontSize(float size) { m_fontSize = size; }

		TextHorizontalAlignment GetHorizontalAlignment() const { return m_horizontalAlignment; }
		void SetHorizontalAlignment(TextHorizontalAlignment alignment) { m_horizontalAlignment = alignment; }

		TextVerticalAlignment GetVerticalAlignment() const { return m_verticalAlignment; }
		void SetVerticalAlignment(TextVerticalAlignment alignment) { m_verticalAlignment = alignment; }

		std::shared_ptr<Font> GetFont() const { return m_font; }
		void SetFont(const std::shared_ptr<Font>& font);

		vec2 MeasureLocalSizeFixed() const;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		std::string m_text = "Text";
		vec4 m_color = vec4(1.0f);
		float m_scale = 1.0f;

		TextSizeMode m_sizeMode = TextSizeMode::AutoSize;
		float m_fontSize = 24.0f;
		TextHorizontalAlignment m_horizontalAlignment = TextHorizontalAlignment::Left;
		TextVerticalAlignment m_verticalAlignment = TextVerticalAlignment::Top;

		std::shared_ptr<Font> m_font;
	};
}