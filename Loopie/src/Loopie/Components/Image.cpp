#include "Image.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Resources/Types/Texture.h"

void Loopie::Image::Init()
{
	if (!GetOwner()->HasComponent<RectTransform>())
		GetOwner()->ReplaceTransform<RectTransform>();

	if (!m_texture)
		SetTexture(Texture::GetDefault());
}

void Loopie::Image::SetTexture(const std::shared_ptr<Texture>& texture)
{
	if (m_texture == texture)
		return;

	if (m_texture)
		m_texture->DecrementReferenceCount();

	m_texture = texture;
	
	if (m_texture)
		m_texture->IncrementReferenceCount();
}

std::shared_ptr<Loopie::Texture> Loopie::Image::GetTexture()
{
	if (m_texture)
		return m_texture;
	else {
		SetTexture(Texture::GetDefault());
		return m_texture;
	}
}

Loopie::JsonNode Loopie::Image::Serialize(JsonNode& parent) const
{
	JsonNode imgNode = parent.CreateObjectField("image");

	JsonNode colorObj = imgNode.CreateObjectField("tint");
	colorObj.CreateField<float>("r", m_tint.r);
	colorObj.CreateField<float>("g", m_tint.g);
	colorObj.CreateField<float>("b", m_tint.b);
	colorObj.CreateField<float>("a", m_tint.a);

	if (m_texture)
		imgNode.CreateField<std::string>("texture_uuid", m_texture->GetUUID().Get());
	
	return imgNode;
}

void Loopie::Image::Deserialize(const JsonNode& data)
{
	m_tint.r = data.GetValue<float>("r", 1.0f).Result;
	m_tint.g = data.GetValue<float>("g", 1.0f).Result;
	m_tint.b = data.GetValue<float>("b", 1.0f).Result;
	m_tint.a = data.GetValue<float>("a", 1.0f).Result;

	if (data.Contains("texture_uuid"))
	{
		UUID texUUID = data.GetValue<std::string>("texture_uuid").Result;
		
		Metadata* meta = AssetRegistry::GetMetadata(texUUID);
		if (meta)
			SetTexture(ResourceManager::GetTexture(*meta));
	}
}
