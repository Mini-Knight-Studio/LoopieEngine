#include "Image.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"

namespace Loopie
{
	Image::~Image()
	{
		if (m_texture)
			m_texture->DecrementReferenceCount();
	}

	void Image::Init()
	{
		if (!GetOwner()->HasComponent<RectTransform>())
			GetOwner()->ReplaceTransform<RectTransform>();

		if (!m_texture)
			SetTexture(Texture::GetDefault());
	}

	void Image::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		if (m_texture == texture)
			return;

		if (m_texture)
			m_texture->DecrementReferenceCount();

		m_texture = texture;

		if (m_texture)
			m_texture->IncrementReferenceCount();
	}

	std::shared_ptr<Texture> Image::GetTexture() const
	{
		return m_texture ? m_texture : Texture::GetDefault();
	}

	JsonNode Image::Serialize(JsonNode& parent) const
	{
		JsonNode imageObj = parent.CreateObjectField("image");

		imageObj.CreateField<float>("r", m_color.r);
		imageObj.CreateField<float>("g", m_color.g);
		imageObj.CreateField<float>("b", m_color.b);
		imageObj.CreateField<float>("a", m_color.a);

		if (m_texture)
			imageObj.CreateField<std::string>("texture_uuid", m_texture->GetUUID().Get());

		return imageObj;
	}

	void Image::Deserialize(const JsonNode& data)
	{
		m_color.r = data.GetValue<float>("r", 1.0f).Result;
		m_color.g = data.GetValue<float>("g", 1.0f).Result;
		m_color.b = data.GetValue<float>("b", 1.0f).Result;
		m_color.a = data.GetValue<float>("a", 1.0f).Result;

		if (data.Contains("texture_uuid"))
		{
			UUID id = UUID(data.GetValue<std::string>("texture_uuid").Result);
			Metadata* meta = AssetRegistry::GetMetadata(id);
			if (meta)
				SetTexture(ResourceManager::GetTexture(*meta));
		}
	}
}