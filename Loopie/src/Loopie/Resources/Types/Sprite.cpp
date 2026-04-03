#include "Sprite.h"

#include "Loopie/Resources/Types/Texture.h"

namespace Loopie
{
	Sprite::Sprite(const UUID& id)
		: Resource(id, ResourceType::SPRITE)
	{
	}

	std::shared_ptr<Sprite> Sprite::Create(const std::shared_ptr<Texture>& texture, const vec4& uvRect)
	{
		std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(UUID());
		sprite->m_texture = texture;
		sprite->m_uvRect = uvRect;
		return sprite;
	}

	bool Sprite::Load()
	{
		if (!m_texture)
			return false;

		// Sprite itself has no cache; ensure the underlying texture is loaded.
		return m_texture->Load();
	}
}
