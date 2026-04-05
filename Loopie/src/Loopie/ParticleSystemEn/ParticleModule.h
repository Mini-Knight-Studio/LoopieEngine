#pragma once
#include <vector>
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/ParticleSystemEn/BillBoardComponent.h"
#include <memory>

namespace Loopie
{

	class ParticleModule
	{
		private:
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
			vec3 m_localOffset; 
			vec3 m_emitterPos; 
			bool m_followEmitter;
			vec3 m_velocityOffset;
			std::shared_ptr<Texture> m_sprite = nullptr;

		public:

			ParticleModule();
			void Update(float dt);
			void Render(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, const matrix4& billboardRotation);

			vec3 GetPosition() const;
			void SetPosition(const vec3& pos);

			vec3 GetVelocity() const;
			void SetVelocity(const vec3& vel);

			vec3 GetVelocityOffset() const;
			void SetVelocityOffset(const vec3& velOffset);

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

			vec3 GetLocalOffset() const;
			void SetLocalOffset(const vec3& offset);

			vec3 GetEmitterPosition() const;
			void SetEmitterPosition(const vec3& pos);

			bool GetFollowEmitter() const;
			void SetFollowEmitter(bool follow);

			std::shared_ptr<Texture> GetSprite() const;
			void SetSprite(std::shared_ptr<Texture> sprite);
	};
}
