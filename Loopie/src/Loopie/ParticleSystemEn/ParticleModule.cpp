#include "ParticleModule.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Profiler/Profiler.h"

namespace Loopie
{
	ParticleModule::ParticleModule()
	{
		m_position = vec3(0, 0, 0);
		m_rotation = vec3(0.0f);
		m_rotationSpeed = vec3(0.0f);
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
		m_rotation += m_rotationSpeed * dt;

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
	// Inside ParticleModule.cpp
	void ParticleModule::Render(const matrix4& billboardRotation, const vec3& emitterScale, const vec3& camPos, std::shared_ptr<Material> material, std::shared_ptr<Texture> sprite, std::shared_ptr<VertexArray> vao)
	{
		LP_FUNC();
		if (!m_active)
		{
			return;
		}

		float life = m_lifeRemaining / m_lifetime;
		if (life <= 0) { life = 0; }

		vec4 color = mix(m_colorEnd, m_colorBegin, life);
		float size = mix(m_sizeEnd, m_sizeBegin, life);

		// transform 
		vec3 finalScale = vec3(size, size, 1.0f) * emitterScale;
		matrix4 transform = translate(matrix4(1.0f), m_position) * billboardRotation;

		transform = rotate(transform, m_rotation.x, vec3(1.0f, 0.0f, 0.0f));
		transform = rotate(transform, m_rotation.y, vec3(0.0f, 1.0f, 0.0f));
		transform = rotate(transform, m_rotation.z, vec3(0.0f, 0.0f, 1.0f));

		transform = scale(transform, finalScale);

		// Calculate squared distance to camera for depth sorting
		vec3 toCam = camPos - m_position;
		float depth = dot(toCam, toCam);

		// Queue it up globally
		Renderer::AddParticle(transform, color, depth, material, sprite, vao);
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

	vec3 ParticleModule::GetRotation() const
	{
		return degrees(m_rotation);
	}
	void ParticleModule::SetRotation(const vec3& rot)
	{
		m_rotation = radians(rot);
	}

	vec3 ParticleModule::GetRotationSpeed() const
	{
		return degrees(m_rotationSpeed);
	}
	void ParticleModule::SetRotationSpeed(const vec3& speed)
	{
		m_rotationSpeed = radians(speed);
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
}
