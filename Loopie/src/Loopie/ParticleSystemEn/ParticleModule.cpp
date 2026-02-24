#include "ParticleModule.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"
namespace Loopie
{
	ParticleModule::ParticleModule()
	{
		m_partType = SMOKE;
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
		m_position += m_velocity * dt;
		m_rotation += 0.01 * dt;

	}
	void ParticleModule::Render(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, const matrix4& billboardTransform)
	{
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
			matrix4 transform = billboardTransform;
			transform = translate(transform, m_position);
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
	
	
	ParticleType ParticleModule::GetParticleType()const
	{
		return m_partType;
	}
	void ParticleModule::SetParticleType(ParticleType t)
	{
		m_partType = t;
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
	}
}
