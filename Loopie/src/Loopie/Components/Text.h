#pragma once

#include "Loopie/Components/UIElement.h"
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

	enum class TextWrapMode
	{
		NoWrap,
		Wrap
	};

	class Text : public UIElement
	{
	public:
		DEFINE_TYPE(Text)

		Text() = default;
		~Text();

		void Init() override;

		void RenderGizmo() const override;

		const std::string& GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; }

		const vec4& GetColor() const { return m_color; }
		void SetColor(const vec4& color) { m_color = color; }

		float GetScale() const { return m_scale; }
		void SetScale(float scale) { m_scale = scale; }

		TextSizeMode GetSizeMode() const { return m_sizeMode; }
		void SetSizeMode(TextSizeMode mode) { m_sizeMode = mode; }

		TextWrapMode GetWrapMode() const { return m_wrapMode; }
		void SetWrapMode(TextWrapMode mode) { m_wrapMode = mode; }

		float GetFontSize() const { return m_fontSize; }
		void SetFontSize(float size) { m_fontSize = size; }

		float GetLineSpacing() const { return m_lineSpacing; }
		void SetLineSpacing(float spacing) { m_lineSpacing = spacing; }

		float GetWordSpacing() const { return m_wordSpacing; }
		void SetWordSpacing(float spacing) { m_wordSpacing = spacing; }

		float GetLetterSpacing() const { return m_letterSpacing; }
		void SetLetterSpacing(float spacing) { m_letterSpacing = spacing; }

		TextHorizontalAlignment GetHorizontalAlignment() const { return m_horizontalAlignment; }
		void SetHorizontalAlignment(TextHorizontalAlignment alignment) { m_horizontalAlignment = alignment; }

		TextVerticalAlignment GetVerticalAlignment() const { return m_verticalAlignment; }
		void SetVerticalAlignment(TextVerticalAlignment alignment) { m_verticalAlignment = alignment; }

		std::shared_ptr<Font> GetFont() const { return m_font; }
		void SetFont(const std::shared_ptr<Font>& font);

		vec2 MeasureLocalSizeFixed() const;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

	private:
		std::string m_text = "Text";
		vec4 m_color = vec4(1.0f);
		float m_scale = 1.0f;

		TextSizeMode m_sizeMode = TextSizeMode::AutoSize;
		TextWrapMode m_wrapMode = TextWrapMode::NoWrap;
		float m_fontSize = 24.0f;
		float m_lineSpacing = 0.0f;
		float m_wordSpacing = 0.0f;
		float m_letterSpacing = 0.0f;
		TextHorizontalAlignment m_horizontalAlignment = TextHorizontalAlignment::Left;
		TextVerticalAlignment m_verticalAlignment = TextVerticalAlignment::Top;

		std::shared_ptr<Font> m_font;
	};
}