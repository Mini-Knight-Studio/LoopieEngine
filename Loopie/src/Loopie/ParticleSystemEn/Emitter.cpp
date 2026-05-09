#include "Emitter.h"
#include "Loopie/ParticleSystemEn/ParticleModule.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Core/Random.h"
#include "Loopie/Math/MathUtils.h"

#include "Loopie/Profiler/Profiler.h"

namespace Loopie
{
	
	float RandomFloat(float min, float max)
	{	
		return FRandom::Get(min, max);
	}

	Emitter::Emitter(unsigned int maxParticles, BillboardType bType, vec3 position, unsigned int spawnRate, vec3 posOffSet)
	{
		m_billboard = Billboard(position, bType);
		m_spawnRate = spawnRate;
	    m_maxParticles = maxParticles;
		m_emitterTimer = 0;
		m_position = position;
		m_positionOffSet = posOffSet;
		m_active = true;
		m_particleFollowEmitter = true;
		m_rotation = quaternion(1, 0, 0, 0);
		m_scale = vec3(1);
		m_localVelocity = true;
		m_poolIndex = 0;
		m_followOwner = true;

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

		m_useSprite.type = UniformType_bool;

	}
	void Emitter::OnUpdate(float dt, bool active)
	{
	
		for (auto& particle : m_particlePool)
		{
			if (!particle.GetActive())
				continue;

			if (m_particleFollowEmitter)
			{
				particle.SetEmitterPosition(m_position);
			}

			particle.Update(dt);
		}
		if ((m_active && active) && m_spawnRate > 0)
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
	void Emitter::OnRender(std::shared_ptr<VertexArray>& quadVAO, std::shared_ptr<Material>& material, Camera* cam)
	{
		LP_FUNC();
		if (!cam)
		{
			Log::Error("No camera passed to particle billboard!");
			return;
		}

		m_billboard.SetPosition(m_position);
		matrix4 billboardRotation = m_billboard.UpdateCalcRotation(cam, m_rotation);
		vec3 scale = m_applyScale ? m_scale : vec3(1);

		vec3 camPos = cam->GetTransform()->GetWorldPosition();


		for (auto it = m_particlePool.begin(); it != m_particlePool.end(); ++it)
		{
			if (!it->GetActive())
				continue;

			it->Render(billboardRotation, scale, camPos, material, m_sprite, quadVAO);
		}
	}
	void Emitter::Emit(const ParticleProps& particleProps)
	{
		ParticleModule& particle = m_particlePool[m_poolIndex];
		vec3 scale = m_applyScale ? m_scale : vec3(1);

		particle.SetActive(true);
		particle.SetVelocityOffset(vec3(0.0f));
		particle.SetRotation(RandomFloat(0, (2 * Math::PI)));

		//Position
		vec3 spawnOffset = m_positionOffSet;
		spawnOffset.x += RandomFloat(-particleProps.PositionVariation.x, particleProps.PositionVariation.x);
		spawnOffset.z += RandomFloat(-particleProps.PositionVariation.z, particleProps.PositionVariation.z);
		spawnOffset.y += RandomFloat(-particleProps.PositionVariation.y, particleProps.PositionVariation.y);
		
		spawnOffset = (m_applyRotation ? m_rotation : quaternion(1,0,0,0)) * (spawnOffset * scale);

		if (m_particleFollowEmitter)
		{
			particle.SetFollowEmitter(true);
			particle.SetLocalOffset(spawnOffset);
			particle.SetEmitterPosition(m_position); 
			particle.SetPosition(m_position + spawnOffset);
		}
		else
		{
			particle.SetFollowEmitter(false);
			particle.SetLocalOffset(vec3(0.0f));
			particle.SetPosition(particleProps.Position + spawnOffset);
		}

		//velocity
		vec3 finalVelocity = particleProps.Velocity;
		finalVelocity.x += RandomFloat(-particleProps.VelocityVariation.x * 1.5f, particleProps.VelocityVariation.x * 1.5f);
		finalVelocity.y += RandomFloat(-particleProps.VelocityVariation.y * 1.5f, particleProps.VelocityVariation.y * 1.5f);
		finalVelocity.z += RandomFloat(-particleProps.VelocityVariation.z * 1.5f, particleProps.VelocityVariation.z * 1.5f);


		if (m_localVelocity)
		{
			finalVelocity = m_rotation * finalVelocity;
		}

		particle.SetVelocity(finalVelocity);

		//color
		particle.SetColorBegin(particleProps.ColorBegin);
		particle.SetColorEnd(particleProps.ColorEnd);

		//size
		float sizeBegin = particleProps.SizeBegin + RandomFloat(-particleProps.SizeVariation * 0.5f, particleProps.SizeVariation * 0.5f);
		particle.SetSizeBegin(sizeBegin);
		particle.SetSizeEnd(particleProps.SizeEnd);
		particle.SetLifetime(particleProps.LifeTime);

		m_poolIndex = (m_poolIndex == 0) ? m_particlePool.size() - 1 : m_poolIndex - 1;
	}
	const std::string& Emitter::GetName() const
	{
		return m_name;
	}
	void Emitter::SetName(const std::string& n)
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
	bool Emitter::GetIsActive() const
	{
		return m_active;
	}
	void Emitter::SetActive(bool isActive) 
	{
		m_active = isActive;
	}

	bool Emitter::GetParticlesFollowEmitter() const
	{
		return m_particleFollowEmitter;
	}
	void Emitter::SetParticlesFollowEmitter(bool follow)
	{
		m_particleFollowEmitter = follow;
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
	std::shared_ptr<Texture> Emitter::GetSprite() const 
	{
		return m_sprite;
	}
	void Emitter::SetSprite(std::shared_ptr<Texture> sprite) 
	{ 
		m_sprite = sprite; 
	}
	const quaternion& Emitter::GetEmitterRotation() const { return m_rotation; }
	void Emitter::SetEmitterRotation(const quaternion& rot) { m_rotation = rot; }

	const vec3& Emitter::GetEmitterScale() const
	{
		return m_scale;
	}

	void Emitter::SetEmitterScale(const vec3& scale)
	{
		m_scale = scale;
	}

	bool Emitter::GetLocalVelocity() const { return m_localVelocity; }
	void Emitter::SetLocalVelocity(bool local) { m_localVelocity = local; }

	bool Emitter::GetIfApplyScale() const
	{
		return m_applyScale;
	}

	void Emitter::SetIfApplyScale(bool apply)
	{
		m_applyScale = apply;
	}

	bool Emitter::GetIfApplyRotation() const
	{
		return m_applyRotation;
	}

	void Emitter::SetIfApplyRotation(bool apply)
	{
		m_applyRotation = apply;
	}

	bool Emitter::GetIsFollowingOwner() const { return m_followOwner; }
	void Emitter::SetFollowingOwner(bool follow) { m_followOwner = follow; }
	Billboard& Emitter::GetBillboard()
	{
		return m_billboard;
	}
}