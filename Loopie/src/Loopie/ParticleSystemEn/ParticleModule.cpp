#include "ParticleModule.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Profiler/Profiler.h"

namespace Loopie
{
	ParticleModule::ParticleModule()
	{
		m_position = vec3(0, 0, 0);
		m_rotation = 0;
		m_velocity = vec3(0, 0, 0);
		m_colorBegin = vec4(1, 1, 1, 1);
		m_colorEnd = vec4(1, 1, 1, 1);
		m_sizeBegin = 1;
		m_sizeEnd = 1;
		m_lifetime = 1;
		m_lifeRemaining = 0;
		m_active = false;
		m_localOffset = vec3(0.0f);
		m_emitterPos = vec3(0.0f);
		m_followEmitter = false;
		m_velocityOffset = vec3(0.0f);
	}
	
	void ParticleModule::Update(float dt)
	{
		if (!m_active)
		{
			return;
		}
		if (m_lifeRemaining <= 0.0f)
		{
			m_active = false;
			return;
		}

		m_lifeRemaining -= dt;
		m_rotation += 0.01 * dt;

		if (m_followEmitter)
		{

			m_velocityOffset += m_velocity * dt;
			m_position = m_emitterPos + m_localOffset + m_velocityOffset;
		}
		else
		{
			m_position += m_velocity * dt;
		}

	}
	void ParticleModule::Render(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, const matrix4& billboardRotation)
	{
		LP_FUNC();
		if (!m_active)
		{
			return;
		}
		
		//interpolations
		float life = m_lifeRemaining / m_lifetime;
		if (life <=0) { life = 0; }

		vec4 color;
		color.r = mix(m_colorEnd.r, m_colorBegin.r, life);
		color.g = mix(m_colorEnd.g, m_colorBegin.g, life);
		color.b = mix(m_colorEnd.b, m_colorBegin.b, life);
		color.a = mix(m_colorEnd.a, m_colorBegin.a, life);

		float size = mix(m_sizeEnd, m_sizeBegin, life);

		// transform 
		matrix4 transform = matrix4(1.0f);
		transform = translate(transform, m_position);         
		transform = transform * billboardRotation;
		transform = rotate(transform, m_rotation, vec3(0.0f, 0.0f, 1.0f));
		transform = scale(transform, vec3(size, size, 1.0f));


		//material
		UniformValue colorUni;
		colorUni.type = UniformType_vec4;
		colorUni.value = color;
		material->SetShaderVariable("u_Color", colorUni);

		//AddParticleRenderItem - > If max capacity reached, flush (this inside AddParticle function), draw and clear pos and color vectors
		Renderer::FlushRenderItem(quadVAO, material, transform);
	}
	
	vec3 ParticleModule::GetPosition() const
	{
		return m_position;
	}
	void ParticleModule::SetPosition(const vec3& pos)
	{
		m_position = pos;
	}

	vec3 ParticleModule::GetVelocity() const
	{ 
		return m_velocity; 
	}
	void ParticleModule::SetVelocity(const vec3& vel)
	{
		m_velocity = vel; 
	}
	vec3 ParticleModule::GetVelocityOffset() const
	{
		return m_velocityOffset;
	}
	void ParticleModule::SetVelocityOffset(const vec3& velOffset)
	{
		m_velocityOffset = velOffset;
	}
	float ParticleModule::GetRotation() const
	{
		return m_rotation;
	}
	void ParticleModule::SetRotation(float rot)
	{
		m_rotation = rot;
	}

	vec4 ParticleModule::GetColorBegin() const
	{
		return m_colorBegin;
	}
	void ParticleModule::SetColorBegin(const vec4& col)
	{
		m_colorBegin = col;
	}

	vec4 ParticleModule::GetColorEnd() const
	{
		return m_colorEnd;
	}
	void ParticleModule::SetColorEnd(const vec4& col)
	{
		m_colorEnd = col;
	}

	float ParticleModule::GetSizeBegin() const
	{
		return m_sizeBegin;
	}
	void ParticleModule::SetSizeBegin(float size)
	{
		m_sizeBegin = size;
	}

	float ParticleModule::GetSizeEnd() const
	{
		return m_sizeEnd;
	}
	void ParticleModule::SetSizeEnd(float size)
	{
		m_sizeEnd = size;
	}

	float ParticleModule::GetLifetime() const
	{
		return m_lifetime;
	}
	void ParticleModule::SetLifetime(float time)
	{
		m_lifetime = time;
		m_lifeRemaining = time;
	}

	float ParticleModule::GetLifeRemaining() const
	{
		return m_lifeRemaining;
	}
	void ParticleModule::SetLifeRemaining(float L_remain) 
	{
		m_lifeRemaining = L_remain;
	}
	bool ParticleModule::GetActive()const
	{
		return m_active;
	}
	void ParticleModule::SetActive(bool act)
	{
		m_active = act;
		if (!act)
		{
			m_velocityOffset = vec3(0.0f);
			m_followEmitter = false;
			m_localOffset = vec3(0.0f);
			m_sprite = nullptr;
		}
	}
	vec3 ParticleModule::GetLocalOffset() const 
	{
		return m_localOffset; 
	}
	void ParticleModule::SetLocalOffset(const vec3& offset) 
	{ 
		m_localOffset = offset;
	}

	vec3 ParticleModule::GetEmitterPosition() const 
	{ 
		return m_emitterPos; 
	}
	void ParticleModule::SetEmitterPosition(const vec3& pos) 
	{ 
		m_emitterPos = pos;
	}

	bool ParticleModule::GetFollowEmitter() const 
	{
		return m_followEmitter;
	}
	void ParticleModule::SetFollowEmitter(bool follow)
	{ 
		m_followEmitter = follow; 
	}
	std::shared_ptr<Texture> ParticleModule::GetSprite() const
	{
		return m_sprite;
	}
	void ParticleModule::SetSprite(std::shared_ptr<Texture> sprite)
	{
		m_sprite = sprite;
	}
}
