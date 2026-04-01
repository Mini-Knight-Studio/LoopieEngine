#pragma once

#include "Loopie/Components/UIElement.h"
#include "Loopie/Math/MathTypes.h"

#include <memory>

namespace Loopie
{
	class Texture;

	class Image : public UIElement
	{
	public:
		DEFINE_TYPE(Image)

		Image() = default;
		~Image();

		void Init() override;

		void RenderGizmo() const override;

		const vec4& GetTint() const { return m_tint; }
		void SetTint(const vec4& tint) { m_tint = tint; }

		std::shared_ptr<Texture> GetTexture();
		void SetTexture(const std::shared_ptr<Texture>& texture);

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		vec4 m_tint = vec4(1.0f);
		std::shared_ptr<Texture> m_texture;
	};
}