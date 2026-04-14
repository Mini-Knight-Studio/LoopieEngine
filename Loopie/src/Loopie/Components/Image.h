#pragma once

#include "Loopie/Components/UIElement.h"
#include "Loopie/Math/MathTypes.h"

#include <memory>

namespace Loopie
{
	class Texture;
	class Sprite;

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

		const vec4& GetUVRect() const { return m_uvRect; }
		void SetUVRect(const vec4& uvRect);

		std::shared_ptr<Sprite> GetSprite() const { return m_sprite; }
		void SetSprite(const std::shared_ptr<Sprite>& sprite);

		std::shared_ptr<Texture> GetTexture();
		void SetTexture(const std::shared_ptr<Texture>& texture);

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

	private:
		vec4 m_tint = vec4(1.0f);
		vec4 m_uvRect = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		std::shared_ptr<Sprite> m_sprite;
		std::shared_ptr<Texture> m_texture;
	};
}