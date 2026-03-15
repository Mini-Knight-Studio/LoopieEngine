#include "Light.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"

namespace Loopie
{

	Light::Light(vec3 color, float intensity, LightType type)
	{
		m_color = color;
		m_intensity = intensity;
		m_type = type;
		Renderer::RegisterLight(this);
	}

	Light::~Light()
	{
		Renderer::UnregisterLight(this);
	}

	void Light::Init()
	{
		switch (m_type)
		{
		case LightType::Ambient:
			break;
		default:
			Log::Warn("Light didn't have any type. Defaulting to Directional Light.");
		case LightType::Directional:
			break;
		case LightType::Spot:
			m_innerConeAngle = 10.0f;
			m_outerConeAngle = 30.0f;
		case LightType::Point:
			m_attenuationConstant = 0.1f;
			m_attenuationLinear = 2.0f;
			m_attenuationQuadratic = 2.0f;
			m_reachDistance = 10.0f;
			break;
		}
	}

	void Light::SetType(LightType lightType)
	{
		m_type = lightType;
	}

	void Light::SetType(int lightType)
	{
		m_type = (LightType)lightType;
	}

	LightType Light::GetLightType() const
	{
		return m_type;
	}

	void Light::SetColor(vec3 color)
	{
		m_color = color;
	}

	vec3 Light::GetColor() const
	{
		return m_color;
	}

	void Light::SetRedColor(float red)
	{
		m_color.x = red;
	}

	float Light::GetRedColor() const
	{
		return m_color.x;
	}

	void Light::SetGreenColor(float green)
	{
		m_color.y = green;
	}

	float Light::GetGreenColor() const
	{
		return m_color.y;
	}

	void Light::SetBlueColor(float blue)
	{
		m_color.z = blue;
	}
	
	float Light::GetBlueColor() const
	{
		return m_color.z;
	}

	void Light::SetIntensity(float intensity)
	{
		m_intensity = std::max(intensity, 0.0f);
	}
	
	float Light::GetIntensity() const
	{
		return m_intensity;
	}

	void Light::SetAttenuationConstant(float attenuationConstant)
	{
		m_attenuationConstant = attenuationConstant;
	}

	float Light::GetAttenuationConstant() const
	{
		return m_attenuationConstant;
	}

	void Light::SetAttenuationLinear(float attenuationLinear)
	{
		m_attenuationLinear = attenuationLinear;
	}

	float Light::GetAttenuationLinear() const
	{
		return m_attenuationLinear;
	}

	void Light::SetAttenuationQuadratic(float attenuationQuadratic)
	{
		m_attenuationQuadratic = attenuationQuadratic;
	}

	float Light::GetAttenuationQuadratic() const
	{
		return m_attenuationQuadratic;
	}

	void Light::SetReachDistance(float fadeDistance)
	{
		m_reachDistance = fadeDistance;
	}

	float Light::GetReachDistance() const
	{
		return m_reachDistance;
	}

	void Light::SetInnerConeAngle(float innerConeAngle)
	{
		m_innerConeAngle = innerConeAngle;
	}
	
	float Light::GetInnerConeAngle() const
	{
		return m_innerConeAngle;
	}

	void Light::SetOuterConeAngle(float outerConeAngle)
	{
		m_outerConeAngle = outerConeAngle;
	}

	float Light::GetOuterConeAngle() const
	{
		return m_outerConeAngle;
	}

	JsonNode Light::Serialize(JsonNode& parent) const
	{
		JsonNode lightObj = parent.CreateObjectField("light");
		lightObj.CreateField<int>("type", (int)m_type);
		JsonNode node = lightObj.CreateObjectField("color");
		node.CreateField("r", m_color.x);
		node.CreateField("g", m_color.y);
		node.CreateField("b", m_color.z);
		lightObj.CreateField<float>("intensity", m_intensity);
		lightObj.CreateField<float>("attenuation_constant", m_attenuationConstant);
		lightObj.CreateField<float>("attenuation_linear", m_attenuationLinear);
		lightObj.CreateField<float>("attenuation_quadratic", m_attenuationQuadratic);
		lightObj.CreateField<float>("reach_distance", m_reachDistance);
		lightObj.CreateField<float>("inner_cone_angle", m_innerConeAngle);
		lightObj.CreateField<float>("outer_cone_angle", m_outerConeAngle);

		return lightObj;
	}

	void Light::Deserialize(const JsonNode& data)
	{
		m_type = (LightType)data.GetValue<int>("type", (int)LightType::Ambient).Result;
		JsonNode node = data.Child("color");
		if (node.IsValid() && node.IsObject())
		{
			m_color.x = node.GetValue<float>("r", 1.0f).Result;
			m_color.y = node.GetValue<float>("g", 1.0f).Result;
			m_color.z = node.GetValue<float>("b", 1.0f).Result;
		}
		m_intensity = data.GetValue<float>("intensity", 0.5f).Result;
		m_attenuationConstant = data.GetValue<float>("attenuation_constant", 0.1f).Result;
		m_attenuationLinear = data.GetValue<float>("attenuation_linear", 2.0f).Result;
		m_attenuationQuadratic = data.GetValue<float>("attenuation_quadratic", 2.0f).Result;
		m_reachDistance = data.GetValue<float>("reach_distance", 10.0f).Result;
		m_innerConeAngle = data.GetValue<float>("inner_cone_angle", 0.0f).Result;
		m_outerConeAngle = data.GetValue<float>("outer_cone_angle", 0.0f).Result;
	}

}