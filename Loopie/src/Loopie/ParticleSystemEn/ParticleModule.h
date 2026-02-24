#pragma once
#include <vector>
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/ParticleSystemEn/BillBoardComponent.h"
#include <memory>

namespace Loopie
{
	
	enum ParticleType
	{
		SMOKE,
		FIREWORK
	};

	class ParticleModule
	{
		private:
			ParticleType m_partType;
			vec3 m_position;
			vec3 m_velocity;
			vec4 m_colorBegin;
			vec4 m_colorEnd;
			float m_rotation;
			float m_sizeBegin;
			float m_sizeEnd;
			float m_lifetime;
			float m_lifeRemaining;
			bool m_active;

		public:

			ParticleModule();
			void Update(float dt);
			void Render(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, const matrix4& billboardTransform);

			
			ParticleType GetParticleType()const;
			void SetParticleType(ParticleType t);

			vec3 GetPosition() const;
			void SetPosition(const vec3& pos);

			vec3 GetVelocity() const;
			void SetVelocity(const vec3& vel);

			float GetRotation() const;
			void SetRotation(float rot);

			float GetLifetime() const;
			void SetLifetime(float time);

			float GetLifeRemaining() const;
			void SetLifeRemaining(float L_remain);

			vec4 GetColorBegin() const;
			void SetColorBegin(const vec4& col);

			vec4 GetColorEnd() const;
			void SetColorEnd(const vec4& col);

			float GetSizeBegin() const;
			void SetSizeBegin(float size);

			float GetSizeEnd() const;
			void SetSizeEnd(float size);

			bool GetActive()const;
			void SetActive(bool act);

	};
}
