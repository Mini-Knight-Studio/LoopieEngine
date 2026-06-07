#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"

namespace Loopie
{
	class Fog : public Component
	{
	public:
		DEFINE_TYPE(Fog)
		Fog();
		~Fog();

		void Init() override;

		void SetFogEnabled(bool enabled);
		bool GetFogEnabled() const;

		void SetFogColor(const vec3& color);
		vec3 GetFogColor() const;

		void SetFogStart(float start);
		float GetFogStart() const;

		void SetFogEnd(float end);
		float GetFogEnd() const;

		void SetFogHeightTop(float top);
		float GetFogHeightTop() const;

		void SetFogHeightBottom(float bottom);
		float GetFogHeightBottom() const;

		void SetFogHeightStrength(float heightStrength);
		float GetFogHeightStrength() const;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

	private:		
		vec3 m_fogColor;
		float m_fogStart;
		float m_fogEnd;
		bool m_fogEnabled;
		float m_fogHeightTop;
		float m_fogHeightBottom;
		float m_fogHeightStrength;
	};
}