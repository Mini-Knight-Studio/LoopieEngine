#pragma once

#include "Loopie/Resources/Resource.h"
#include "Loopie/Math/MathTypes.h"

#include <memory>

namespace Loopie
{
	class Texture;

	class Sprite : public Resource
	{
	public:
		DEFINE_TYPE(Sprite)

		Sprite(const UUID& id);
		~Sprite() = default;

		static std::shared_ptr<Sprite> Create(const std::shared_ptr<Texture>& texture, const vec4& uvRect = vec4(0.0f, 0.0f, 1.0f, 1.0f));

		bool Load() override;

		std::shared_ptr<Texture> GetTexture() const { return m_texture; }
		void SetTexture(const std::shared_ptr<Texture>& texture) { m_texture = texture; }

		const vec4& GetUVRect() const { return m_uvRect; }
		void SetUVRect(const vec4& uvRect) { m_uvRect = uvRect; }

	private:
		std::shared_ptr<Texture> m_texture;
		vec4 m_uvRect = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	};
}
