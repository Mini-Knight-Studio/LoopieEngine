#include "Image.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Resources/Types/Texture.h"

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

	void Image::RenderGizmo()
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

		m_texture = texture;
	}

	std::shared_ptr<Texture> Image::GetTexture()
	{
		if (m_texture)
			return m_texture;
		else {
			SetTexture(Texture::GetDefault());
			return m_texture;
		}
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

		return imgNode;
	}

	void Image::Deserialize(const JsonNode& data)
	{
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

		if (data.Contains("texture_uuid"))
		{
			UUID texUUID = data.GetValue<std::string>("texture_uuid").Result;

			Metadata* meta = AssetRegistry::GetMetadata(texUUID);
			if (meta) {
				auto tex = ResourceManager::GetTexture(*meta);
				if (!tex || !tex->Load()) {
					SetTexture(Texture::GetDefault());
					return;
				}
				SetTexture(tex);
				return;
			}
		}

		SetTexture(Texture::GetDefault());
	}

}
