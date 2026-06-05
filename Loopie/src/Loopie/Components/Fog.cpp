#include "Fog.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"

namespace Loopie
{
	Fog::Fog()
	{
		m_fogColor = vec3(0.55f, 0.60f, 0.70f);
		m_fogStart = 220.0f;
		m_fogEnd = 240.0f;
		m_fogEnabled = true;
		m_fogHeightTop = 0.0f;		 // At / above this world Y: no height fog
		m_fogHeightBottom = -50.0f;  // At / below this: full height fog
		m_fogHeightStrength = 1.0f;  // At 0 disables height fog, leaving pure distance fog
		Renderer::RegisterFog(this);
	}

	Fog::~Fog()
	{
		Renderer::UnregisterFog(this);
	}

	void Fog::Init()
	{

	}

	void Fog::SetFogEnabled(bool enabled)
	{
		enabled ? Renderer::RegisterFog(this) : Renderer::UnregisterFog(this);
		m_fogEnabled = enabled;
	}

	bool Fog::GetFogEnabled() const
	{
		return m_fogEnabled;
	}

	void Fog::SetFogColor(const vec3& color)
	{
		m_fogColor = color;
	}

	vec3 Fog::GetFogColor() const
	{
		return m_fogColor;
	}

	void Fog::SetFogStart(float start)
	{
		m_fogStart = start;
	}

	float Fog::GetFogStart() const
	{
		return m_fogStart;
	}

	void Fog::SetFogEnd(float end)
	{
		m_fogEnd = end;
	}

	float Fog::GetFogEnd() const
	{
		return m_fogEnd;
	}

	void Fog::SetFogHeightTop(float top)
	{
		m_fogHeightTop = top;
	}

	float Fog::GetFogHeightTop() const
	{
		return m_fogHeightTop;
	}

	void Fog::SetFogHeightBottom(float bottom)
	{
		m_fogHeightBottom = bottom;
	}

	float Fog::GetFogHeightBottom() const
	{
		return m_fogHeightBottom;
	}

	void Fog::SetFogHeightStrength(float heightStrength)
	{
		m_fogHeightStrength = heightStrength;
	}

	float Fog::GetFogHeightStrength() const
	{
		return m_fogHeightStrength;
	}

	JsonNode Fog::Serialize(JsonNode& parent) const
	{
		JsonNode fogObj = parent.CreateObjectField("fog");
		JsonNode node = fogObj.CreateObjectField("fog_color");
		node.CreateField("r", m_fogColor.x);
		node.CreateField("g", m_fogColor.y);
		node.CreateField("b", m_fogColor.z);
		fogObj.CreateField<float>("fog_start", m_fogStart);
		fogObj.CreateField<float>("fog_end", m_fogEnd);
		fogObj.CreateField<bool>("fog_enabled", m_fogEnabled);
		fogObj.CreateField<float>("fog_height_top", m_fogHeightTop);
		fogObj.CreateField<float>("fog_height_bottom", m_fogHeightBottom);
		fogObj.CreateField<float>("fog_height_strength", m_fogHeightStrength);

		return fogObj;
	}

	void Fog::Deserialize(const JsonNode& data)
	{
		JsonNode node = data.Child("fog_color");
		if (node.IsValid() && node.IsObject())
		{
			m_fogColor.x = node.GetValue<float>("r", 0.55f).Result;
			m_fogColor.y = node.GetValue<float>("g", 0.60f).Result;
			m_fogColor.z = node.GetValue<float>("b", 0.70f).Result;
		}
		m_fogStart = data.GetValue<float>("fog_start", 220.0f).Result;
		m_fogEnd = data.GetValue<float>("fog_end", 240.0f).Result;
		m_fogEnabled = data.GetValue<bool>("fog_enabled", true).Result;
		m_fogHeightTop = data.GetValue<float>("fog_height_top", 0.0f).Result;
		m_fogHeightBottom = data.GetValue<float>("fog_height_bottom", -50.0f).Result;
		m_fogHeightStrength = data.GetValue<float>("fog_height_strength", 1.0f).Result;
	}

	void Fog::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const Fog& otherFog = static_cast<const Fog&>(other);

		m_fogColor = otherFog.m_fogColor;
		m_fogStart = otherFog.m_fogStart;
		m_fogEnd = otherFog.m_fogEnd;
		m_fogEnabled = otherFog.m_fogEnabled;
		m_fogHeightTop = otherFog.m_fogHeightTop;
		m_fogHeightBottom = otherFog.m_fogHeightBottom;
		m_fogHeightStrength = otherFog.m_fogHeightStrength;
	}
}