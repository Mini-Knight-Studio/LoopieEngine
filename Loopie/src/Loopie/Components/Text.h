#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Resources/Types/Font.h"

#include <memory>
#include <string>

namespace Loopie
{
	class Text : public Component
	{
	public:
		DEFINE_TYPE(Text)

		Text() = default;
		~Text() override = default;

		void Init() override;

		void RenderGizmo() override;

		const std::string& GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; }

		const vec4& GetColor() const { return m_color; }
		void SetColor(const vec4& color) { m_color = color; }

		float GetScale() const { return m_scale; }
		void SetScale(float scale) { m_scale = scale; }

		std::shared_ptr<Font> GetFont() const { return m_font; }
		void SetFont(const std::shared_ptr<Font>& font);

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		std::string m_text = "Text";
		vec4 m_color = vec4(1.0f);
		float m_scale = 1.0f;

		std::shared_ptr<Font> m_font;
	};
}