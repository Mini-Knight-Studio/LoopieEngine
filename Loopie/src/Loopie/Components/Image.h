#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Resources/Types/Texture.h"

#include <memory>

namespace Loopie
{
	class Image : public Component
	{
	public:
		DEFINE_TYPE(Image)

		Image() = default;
		~Image() override;

		void Init() override;

		void SetTexture(const std::shared_ptr<Texture>& texture);
		std::shared_ptr<Texture> GetTexture() const;

		void SetColor(const vec4& color) { m_color = color; }
		const vec4& GetColor() const { return m_color; }

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		std::shared_ptr<Texture> m_texture;
		vec4 m_color = vec4(1.0f);
	};
}