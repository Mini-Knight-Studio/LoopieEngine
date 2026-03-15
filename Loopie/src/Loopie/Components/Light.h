#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"

namespace Loopie
{
	enum class LightType
	{
		Ambient = 0, // Lights all the scene equally, no direction (placed on camera)
		Directional, // Has a direction, color and intensity (sun / moon)
		Spot, // Has a position, direction, a cone and attenuation (falloff)
		Point // Has a position and attenuation (falloff), spreads in all directions 
	};

	class Light : public Component
	{
	public:
		DEFINE_TYPE(Light)

		Light(vec3 color = vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f, LightType type = LightType::Directional);
		~Light();

		void Init() override;

		void SetType(LightType lightType);
		void SetType(int lightType);
		LightType GetLightType() const;

		void SetColor(vec3 color);
		vec3 GetColor() const;

		void  SetRedColor(float red);
		float GetRedColor() const;
		void  SetGreenColor(float green);
		float GetGreenColor() const;
		void  SetBlueColor(float blue);
		float GetBlueColor() const;
		void  SetIntensity(float intensity);
		float GetIntensity() const;
		
		void  SetAttenuationConstant(float attenuationConstant);
		float GetAttenuationConstant() const;
		void  SetAttenuationLinear(float attenuationLinear);
		float GetAttenuationLinear() const;
		void  SetAttenuationQuadratic(float attenuationQuadratic);
		float GetAttenuationQuadratic() const;

		void  SetReachDistance(float fadeDistance);
		float GetReachDistance() const;
		void  SetInnerConeAngle(float innerConeAngle);
		float GetInnerConeAngle() const;
		void  SetOuterConeAngle(float outerConeAngle);
		float GetOuterConeAngle() const;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		LightType	m_type;
		vec3		m_color; // Values should range from 1.0 to 0.0
		float		m_intensity;
		
		float m_attenuationConstant;
		float m_attenuationLinear;
		float m_attenuationQuadratic;

		// *** ReachDistance as an optimization *** - PSS 09/03/2026
		// Reach distance is not used. It's an optimization that is thought for later if needed.
		// The reason it's not used is because the UBO layout is packed into 16 scalars * 4 bytes = 64 bytes,
		// which is the perfect size for UBO layouts. 
		// If we need it later, it can be added, even if the memory usage is not optimized.
		float m_reachDistance; 
		float m_innerConeAngle;
		float m_outerConeAngle;
	};
}