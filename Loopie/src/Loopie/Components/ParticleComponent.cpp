#include "ParticleComponent.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/ParticleSystemEn/ParticleSystem.h"
#include "Loopie/ParticleSystemEn/Emitter.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Log.h"

using namespace std;
namespace Loopie
{
	ParticleComponent::ParticleComponent()
	{
		m_partSystem = nullptr;
	}
	ParticleComponent::ParticleComponent(ParticleSystem* pSystem)
	{
		m_partSystem = pSystem;
	}
	void ParticleComponent::Save()
	{

	}
	void ParticleComponent::Load()
	{

	}
	void ParticleComponent::Init()
	{
		GetTransform()->m_transformNotifier.AddObserver(this);
	}
	void ParticleComponent::Update() 
	{
		vec3 pos = GetOwner()->GetComponent<Transform>()->GetPosition();
		vec3 localPos = GetOwner()->GetComponent<Transform>()->GetLocalPosition();
		for (size_t i = 0; i < GetEmittersVector().size(); i++)
		{
			GetEmittersVector()[i]->SetPosition(pos + GetEmittersVector()[i]->GetPositionOffSet());
		}
		
		float dt = (float)Time::GetDeltaTime();
		m_partSystem->OnUpdate(dt);
	}
	void ParticleComponent::Render(Camera* cam)
	{
		m_partSystem->OnRender(cam);
	}
	void ParticleComponent::Reset()
	{

	}

	void ParticleComponent::OnNotify(const TransformNotification& id)
	{
	}

	JsonNode ParticleComponent::Serialize(JsonNode& parent) const
	{
		JsonNode particleObj = parent.CreateObjectField("particlecomponent");
		for (size_t i = 0; i < m_partSystem->GetEmitterArray().size(); i++)
		{
			JsonNode emitterNode = particleObj.CreateObjectField("i");
			emitterNode.CreateField("name", m_partSystem->GetEmitterArray()[i]->GetName());
			emitterNode.CreateField("spawnrate", m_partSystem->GetEmitterArray()[i]->GetSpawnrate());
			emitterNode.CreateField("maxparticles", m_partSystem->GetEmitterArray()[i]->GetMaxParticles());
			emitterNode.CreateField("emmitertimer", m_partSystem->GetEmitterArray()[i]->GetEmitterTimer());
			emitterNode.CreateField("active", m_partSystem->GetEmitterArray()[i]->IsActive());
			emitterNode.CreateField("poolindex", m_partSystem->GetEmitterArray()[i]->GetPoolIndex());

			emitterNode.CreateObjectField("position");
			emitterNode.CreateField("x", m_partSystem->GetEmitterArray()[i]->GetPosition().x);
			emitterNode.CreateField("y", m_partSystem->GetEmitterArray()[i]->GetPosition().y);
			emitterNode.CreateField("z", m_partSystem->GetEmitterArray()[i]->GetPosition().z);

			emitterNode.CreateObjectField("positionoffset");
			emitterNode.CreateField("x", m_partSystem->GetEmitterArray()[i]->GetPositionOffSet().x);
			emitterNode.CreateField("y", m_partSystem->GetEmitterArray()[i]->GetPositionOffSet().y);
			emitterNode.CreateField("z", m_partSystem->GetEmitterArray()[i]->GetPositionOffSet().z);


			JsonNode pProps = emitterNode.CreateObjectField("particleprops");

			pProps.CreateField("sizebegin", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().SizeBegin);
			pProps.CreateField("sizeend", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().SizeEnd);
			pProps.CreateField("sizevariation", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().SizeVariation);
			pProps.CreateField("lifetime", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().LifeTime);

			pProps.CreateObjectField("position");
			pProps.CreateField("x", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().Position.x);
			pProps.CreateField("y", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().Position.y);
			pProps.CreateField("z", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().Position.z);

			pProps.CreateObjectField("positionvariation");
			pProps.CreateField("x", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().PositionVariation.x);
			pProps.CreateField("y", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().PositionVariation.y);
			pProps.CreateField("z", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().PositionVariation.z);

			pProps.CreateObjectField("velocity");
			pProps.CreateField("x", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().Velocity.x);
			pProps.CreateField("y", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().Velocity.y);
			pProps.CreateField("z", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().Velocity.z);

			pProps.CreateObjectField("velocityvariation");
			pProps.CreateField("x", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().VelocityVariation.x);
			pProps.CreateField("y", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().VelocityVariation.y);
			pProps.CreateField("z", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().VelocityVariation.z);

			pProps.CreateObjectField("colorbegin");
			pProps.CreateField("r", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.x);
			pProps.CreateField("g", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.y);
			pProps.CreateField("b", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.z);
			pProps.CreateField("a", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.z);


			pProps.CreateObjectField("colorend");
			pProps.CreateField("r", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.x);
			pProps.CreateField("g", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.y);
			pProps.CreateField("b", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.z);
			pProps.CreateField("a", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.z);
		}
		
		return particleObj;
	}

	void ParticleComponent::Deserialize(const JsonNode& data)
	{
		for (size_t i = 0; i < m_partSystem->GetEmitterArray().size(); i++)
		{
			JsonNode node = data.Child("i");
			if (node.IsValid() && node.IsObject())
			{
				m_partSystem->GetEmitterArray()[i]->SetName(node.GetValue<string>("name").Result);
				m_partSystem->GetEmitterArray()[i]->SetSpawnRate(node.GetValue<unsigned int>("spawnrate").Result);
				m_partSystem->GetEmitterArray()[i]->SetMaxParticles(node.GetValue<unsigned int>("maxparticles").Result);
				m_partSystem->GetEmitterArray()[i]->SetEmitterTimer(node.GetValue<float>("emmitertimer").Result);
				m_partSystem->GetEmitterArray()[i]->SetActive(node.GetValue<bool>("active").Result);
				m_partSystem->GetEmitterArray()[i]->SetPoolIndex(node.GetValue<unsigned int>("poolindex").Result);

			}


		}
	}
	std::vector<Emitter*> ParticleComponent::GetEmittersVector()
	{
		return m_partSystem->GetEmitterArray();
	}
	void ParticleComponent::AddElemToEmitterVector(Emitter* emitter)
	{
		if (m_partSystem != nullptr || emitter != nullptr)
		{
			m_partSystem->AddElemToEmitterArray(emitter);
		}
	}
	ParticleSystem* ParticleComponent::GetParticleSystem()
	{
		return m_partSystem;
	}
	void ParticleComponent::SetParticleSystem(ParticleSystem* pSystem)
	{
		m_partSystem = pSystem;
	}
}