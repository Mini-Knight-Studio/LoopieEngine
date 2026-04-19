#include "Image.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Resources/Types/Texture.h"
#include "Loopie/Resources/Types/Sprite.h"

namespace Loopie
{
	Image::~Image()
	{

	}
	void Image::Init()
	{
		if (!GetOwner()->HasComponent<RectTransform>())
			GetOwner()->ReplaceTransform<RectTransform>();

		if (!m_texture)
			SetTexture(Texture::GetDefault());
	}

	void Image::RenderGizmo() const
	{
		auto* rt = GetOwner() ? GetOwner()->GetComponent<RectTransform>() : nullptr;
		if (!rt)
			return;

		const matrix4& m = rt->GetLocalToWorldMatrix();
		const vec3 localMin = rt->GetLocalBoundsMin();
		const vec3 localMax = rt->GetLocalBoundsMax();

		const vec3 corners[8] =
		{
			{ localMin.x, localMin.y, 0.0f },
			{ localMax.x, localMin.y, 0.0f },
			{ localMin.x, localMax.y, 0.0f },
			{ localMax.x, localMax.y, 0.0f },
			{ localMin.x, localMin.y, 0.0f },
			{ localMax.x, localMin.y, 0.0f },
			{ localMin.x, localMax.y, 0.0f },
			{ localMax.x, localMax.y, 0.0f },
		};

		AABB aabb;
		aabb.SetNegativeInfinity();
		for (int i = 0; i < 4; ++i)
		{
			const vec3 w = vec3(m * vec4(corners[i], 1.0f));
			aabb.Enclose(w);
		}

		Gizmo::DrawCube(aabb, Color::BLUE);
	}

	void Image::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		if (m_texture == texture)
			return;

		m_sprite.reset();
		m_uvRect = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		m_texture = texture;
	}

	void Image::SetUVRect(const vec4& uvRect)
	{
		m_sprite.reset();
		m_uvRect = uvRect;
	}

	void Image::SetSprite(const std::shared_ptr<Sprite>& sprite)
	{
		m_sprite = sprite;
		if (m_sprite)
		{
			m_texture = m_sprite->GetTexture();
			m_uvRect = m_sprite->GetUVRect();
		}
		else
		{
			m_uvRect = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
	}

	std::shared_ptr<Texture> Image::GetTexture()
	{
		if (m_texture)
			return m_texture;

		SetTexture(Texture::GetDefault());
		return m_texture;
	}

	JsonNode Image::Serialize(JsonNode& parent) const
	{
		JsonNode imgNode = parent.CreateObjectField("image");

		JsonNode colorObj = imgNode.CreateObjectField("tint");
		colorObj.CreateField<float>("r", m_tint.r);
		colorObj.CreateField<float>("g", m_tint.g);
		colorObj.CreateField<float>("b", m_tint.b);
		colorObj.CreateField<float>("a", m_tint.a);

		if (m_texture)
			imgNode.CreateField<std::string>("texture_uuid", m_texture->GetUUID().Get());

		JsonNode uvObj = imgNode.CreateObjectField("uv_rect");
		uvObj.CreateField<float>("min_u", m_uvRect.x);
		uvObj.CreateField<float>("min_v", m_uvRect.y);
		uvObj.CreateField<float>("max_u", m_uvRect.z);
		uvObj.CreateField<float>("max_v", m_uvRect.w);

		SerializeDrawOrder(imgNode);

		return imgNode;
	}

	void Image::Deserialize(const JsonNode& data)
	{
		vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

		if (data.Contains("tint"))
		{
			JsonNode colorObj = data.Child("tint");
			if (colorObj.IsValid())
			{
				m_tint.r = colorObj.GetValue<float>("r", 1.0f).Result;
				m_tint.g = colorObj.GetValue<float>("g", 1.0f).Result;
				m_tint.b = colorObj.GetValue<float>("b", 1.0f).Result;
				m_tint.a = colorObj.GetValue<float>("a", 1.0f).Result;
			}
		}
		else
		{
			m_tint = vec4(1.0f);
		}

		if (data.Contains("uv_rect"))
		{
			JsonNode uvObj = data.Child("uv_rect");
			if (uvObj.IsValid())
			{
				uvRect.x = uvObj.GetValue<float>("min_u", 0.0f).Result;
				uvRect.y = uvObj.GetValue<float>("min_v", 0.0f).Result;
				uvRect.z = uvObj.GetValue<float>("max_u", 1.0f).Result;
				uvRect.w = uvObj.GetValue<float>("max_v", 1.0f).Result;
			}
		}

		bool textureSet = false;
		if (data.Contains("texture_uuid"))
		{
			UUID texUUID = data.GetValue<std::string>("texture_uuid").Result;

			Metadata* meta = AssetRegistry::GetMetadata(texUUID);
			if (meta)
			{
				auto tex = ResourceManager::GetTexture(*meta);
				if (tex && tex->Load())
				{
					SetTexture(tex);
					textureSet = true;
				}
			}
		}

		if (!textureSet)
			SetTexture(Texture::GetDefault());

		m_uvRect = uvRect;
		DeserializeDrawOrder(data);
	}

	void Image::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const Image& otherImage = static_cast<const Image&>(other);
		m_tint = otherImage.m_tint;
		m_uvRect = otherImage.m_uvRect;
		SetTexture(otherImage.m_texture);
		CloneDrawOrder(otherImage);
	}

}
