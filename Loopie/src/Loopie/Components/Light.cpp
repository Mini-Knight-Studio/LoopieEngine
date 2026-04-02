#include "Light.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Components/Transform.h"

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

	void Light::RenderGizmo() const {
		switch (m_type)
		{
		case Loopie::LightType::Ambient: {

			break;
		}
		case Loopie::LightType::Directional:
		{
			vec3 pos = GetOwner()->GetTransform()->GetWorldPosition();
			vec3 dir = GetOwner()->GetTransform()->Forward();

			Gizmo::DrawLine(pos, pos + dir * 3.0f, Color::YELLOW);

			vec3 right = glm::normalize(glm::cross(dir, vec3(0, 1, 0)));
			vec3 up = glm::cross(right, dir);

			float arrowSize = 0.5f;

			Gizmo::DrawLine(pos + dir * 3.0f, pos + dir * 3.0f - dir * arrowSize + right * arrowSize, Color::YELLOW);
			Gizmo::DrawLine(pos + dir * 3.0f, pos + dir * 3.0f - dir * arrowSize - right * arrowSize, Color::YELLOW);
			break;
		}
		case Loopie::LightType::Spot:
		{
			vec3 pos = GetOwner()->GetTransform()->GetWorldPosition();
			vec3 dir = GetOwner()->GetTransform()->Forward();

			float length = 5.0f;
			float angle = glm::radians(m_outerConeAngle);
			float radius = tan(angle) * length;

			vec3 end = pos + dir * length;

			Gizmo::DrawCircle(end, radius, dir, 32, Color::YELLOW);

			const int segments = 16;
			constexpr float step = glm::two_pi<float>() / segments;

			vec3 tangent = glm::normalize(glm::cross(dir, vec3(0, 1, 0)));
			if (glm::length(tangent) < 0.001f)
				tangent = glm::normalize(glm::cross(dir, vec3(1, 0, 0)));
			vec3 bitangent = glm::cross(dir, tangent);

			for (int i = 0; i < segments; i++)
			{
				float a = step * i;
				vec3 point = end + radius * (cos(a) * tangent + sin(a) * bitangent);

				Gizmo::DrawLine(pos, point, Color::YELLOW);
			}
			break;
		}		
		case Loopie::LightType::Point:{
			vec3 pos = GetOwner()->GetTransform()->GetWorldPosition();
			Gizmo::DrawSphere(pos, 1.0f, 32, Color::YELLOW);
			break;
		}	
		default:
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

	// Creates a matrix from the light pov, is used to produce shadows
	matrix4 Light::GetLightSpaceMatrix(const vec3& sceneCenter, float orthoSize, float nearPlane, float farPlane) const
	{
		matrix4 viewMat;
		matrix4 projMatrix;

		switch (m_type)
		{
		default:
		case LightType::Ambient:
			break;
		case LightType::Directional:
		{
			vec3 lightDir = GetTransform()->Forward();
			vec3 lightPos = sceneCenter - lightDir * (farPlane * 0.5f);

			vec3 up = vec3(0.0f, 1.0f, 0.0f); // If the up vector and direction of light are parallel, it produces garbage
			if (abs(dot(lightDir, up)) > 0.99f)
			{
				up = vec3(1.0f, 0.0f, 0.0f);
			}
			viewMat = glm::lookAt(lightPos, sceneCenter, up);
			projMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
			break;
		}
		case LightType::Spot: 
		{
			vec3 lightDir = GetTransform()->Forward();
			vec3 lightPos = GetTransform()->GetWorldPosition();

			vec3 up = vec3(0.0f, 1.0f, 0.0f); // If the up vector and direction of light are parallel, it produces garbage
			if (abs(dot(lightDir, up)) > 0.99f)
			{
				up = vec3(1.0f, 0.0f, 0.0f);
			}
			viewMat = glm::lookAt(lightPos, lightPos + lightDir, up);
			// Modify the nearPlane value below to play with the precision of shadows (how far shadows are casted)
			projMatrix = glm::perspective(radians(m_outerConeAngle * 2.0f), 1.0f, 3.0f, farPlane); 
			break;
		}
		case LightType::Point:
			// TODO
			break;
		}

		return projMatrix * viewMat;
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