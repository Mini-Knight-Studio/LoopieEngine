#include "Emitter.h"
#include "Loopie/ParticleSystemEn/ParticleModule.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Math/MathUtils.h"

namespace Loopie
{
	
	float RandomFloat(float min, float max)
	{
		return min + ((float)rand() / (float)RAND_MAX) * (max - min);
	}

	Emitter::Emitter(unsigned int maxParticles, BillboardType bType, vec3 position, unsigned int spawnRate, vec3 posOffSet)
	{
		m_billboard = std::make_shared<Billboard>(position, bType);
		m_spawnRate = spawnRate;
	    m_maxParticles = maxParticles;
		m_emitterTimer = 0;
		m_position = position;
		m_positionOffSet = posOffSet;
		m_active = true;
		m_poolIndex = 0;

		m_name = "DefaultParticle";
		m_particleProperties.Velocity = vec3(0, 0, 0);
		m_particleProperties.VelocityVariation = vec3(1, 1, 0);
		m_particleProperties.ColorBegin = vec4(1, 1, 1, 1);
		m_particleProperties.ColorEnd = vec4(1, 1, 1, 1);
		m_particleProperties.SizeBegin = 1;
		m_particleProperties.SizeEnd = 1;
		m_particleProperties.SizeVariation = 0.5;
		m_particleProperties.LifeTime = 1;
		
		m_particlePool.resize(m_maxParticles);
		m_poolIndex = m_maxParticles - 1;
	}
	void Emitter::OnUpdate(float dt)
	{
		
		for (auto& particle : m_particlePool)
		{
			if (particle.GetActive() == false)
			{
                continue;
			}
			particle.Update(dt);
		}
		if (m_active && m_spawnRate > 0)
		{
			m_emitterTimer += dt;
			float emissionInterval = 1.0f / m_spawnRate;

			while (m_emitterTimer >= emissionInterval)
			{
				ParticleProps props = m_particleProperties;
				props.Position = m_position;
				Emit(props);
				m_emitterTimer -= emissionInterval;
			}
		}
	
	}
	void Emitter::OnRender(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, Camera* cam)
	{
		if (!cam)
		{
			Log::Error("no camera passed to particle billboard!");
		}

		m_billboard->SetPosition(m_position);
		matrix4 billboardTransform = m_billboard->UpdateCalc(cam);

		for (auto it = m_particlePool.rbegin(); it != m_particlePool.rend(); ++it)
		{
			auto& particle = *it;

			if (!particle.GetActive())
			{
				continue;
			}
				
			particle.Render(quadVAO, material, billboardTransform);
		}
	}
	void Emitter::Emit(const ParticleProps& particleProps)
	{
		ParticleModule& particle = m_particlePool[m_poolIndex];
		
		particle.SetActive(true);
		particle.SetPosition(particleProps.Position);
		particle.SetRotation(RandomFloat(0, (2 *Math::PI)));

		//position
		vec3 position = vec3(0.0f);
		position.x += RandomFloat(-particleProps.PositionVariation.x, particleProps.PositionVariation.x);
		position.z += RandomFloat(-particleProps.PositionVariation.z, particleProps.PositionVariation.z);
		particle.SetPosition(position);

		// velocity
		vec3 finalVelocity = particleProps.Velocity;
		finalVelocity.x += RandomFloat(-particleProps.VelocityVariation.x * 1.5, particleProps.VelocityVariation.x * 1.5);
		finalVelocity.y += RandomFloat(-particleProps.VelocityVariation.y * 1.5, particleProps.VelocityVariation.y * 1.5);
		particle.SetVelocity(finalVelocity);

		// colors
		particle.SetColorBegin(particleProps.ColorBegin);
		particle.SetColorEnd(particleProps.ColorEnd);

		// size
		float sizeBegin = particleProps.SizeBegin + RandomFloat(-particleProps.SizeVariation * 0.5f, particleProps.SizeVariation * 0.5f);
		particle.SetSizeBegin(sizeBegin);
		particle.SetSizeEnd(particleProps.SizeEnd);
		
		particle.SetLifetime(particleProps.LifeTime);
		
		m_poolIndex = (m_poolIndex - 1) % m_particlePool.size();
	}

	std::string Emitter::GetName()const
	{
		return m_name;
	}
	void Emitter::SetName(std::string n)
	{
		m_name = n;
	}

	unsigned int Emitter::GetSpawnrate()const
	{
		return m_spawnRate;
	}
	void Emitter::SetSpawnRate(unsigned int spawnR)
	{
		m_spawnRate = spawnR;
	}

	unsigned int Emitter::GetMaxParticles()const
	{
		return m_maxParticles;
	}
	void Emitter::SetMaxParticles(unsigned int maxPart)
	{
		m_maxParticles = maxPart;
		m_particlePool.resize(m_maxParticles);
		m_poolIndex = m_maxParticles - 1;
	}
	float Emitter::GetEmitterTimer() const
	{
		return m_emitterTimer;
	}
	void Emitter::SetEmitterTimer(float timer)
	{
		m_emitterTimer = timer;
	}
	vec3 Emitter::GetPosition() const
	{
		return m_position;
	}
	void Emitter::SetPosition(const vec3& pos)
	{
		m_position = pos;
	}
	vec3 Emitter::GetPositionOffSet() const
	{
		return m_positionOffSet;
	}
	void Emitter::SetPositionOffSet(const vec3& posOffSet)
	{
		m_positionOffSet = posOffSet;
	}
	int Emitter::GetActiveParticles() const
	{
		int count = 0;
		for (const auto& particle : m_particlePool)
		{
			if (particle.GetActive())
			{
				count++;
			}
				
		}
		return count;
	}
	bool Emitter::IsActive() const
	{
		return m_active;
	}
	void Emitter::SetActive(bool isActive) 
	{
		m_active = isActive;
	}
	void Emitter::ToggleActive()
	{
		m_active = !m_active;
	}
	void Emitter::SetEmisionProperties(const ParticleProps& partProps)
	{
		m_particleProperties = partProps;
	}
	unsigned int Emitter::GetPoolIndex() const
	{
		return m_poolIndex;
	}
	void Emitter::SetPoolIndex(unsigned int poolIndex)
	{
		m_poolIndex = poolIndex;
	}
	ParticleProps& Emitter::GetEmissionProperties()
	{ 
		return m_particleProperties;
	}

}