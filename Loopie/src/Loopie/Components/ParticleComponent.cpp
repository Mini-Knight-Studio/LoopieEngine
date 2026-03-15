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
			pProps.CreateField("a", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.w);


			pProps.CreateObjectField("colorend");
			pProps.CreateField("r", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.x);
			pProps.CreateField("g", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.y);
			pProps.CreateField("b", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.z);
			pProps.CreateField("a", m_partSystem->GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.w);
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

				JsonNode positionNode = node.Child("position");
				if (positionNode.IsValid() && positionNode.IsObject())
				{
					glm::vec3 position;
					position.x = positionNode.GetValue<float>("x").Result;
					position.y = positionNode.GetValue<float>("y").Result;
					position.z = positionNode.GetValue<float>("z").Result;
					m_partSystem->GetEmitterArray()[i]->SetPosition(position);
				}

				JsonNode positionOffsetNode = node.Child("positionoffset");
				if (positionOffsetNode.IsValid() && positionOffsetNode.IsObject())
				{
					glm::vec3 positionOffset;
					positionOffset.x = positionOffsetNode.GetValue<float>("x").Result;
					positionOffset.y = positionOffsetNode.GetValue<float>("y").Result;
					positionOffset.z = positionOffsetNode.GetValue<float>("z").Result;
					m_partSystem->GetEmitterArray()[i]->SetPositionOffSet(positionOffset);
				}

				JsonNode pPropsNode = node.Child("particleprops");
				if (pPropsNode.IsValid() && pPropsNode.IsObject())
				{
					ParticleProps props;

					props.SizeBegin = pPropsNode.GetValue<float>("sizebegin").Result;
					props.SizeEnd = pPropsNode.GetValue<float>("sizeend").Result;
					props.SizeVariation = pPropsNode.GetValue<float>("sizevariation").Result;
					props.LifeTime = pPropsNode.GetValue<float>("lifetime").Result;

					JsonNode propPositionNode = pPropsNode.Child("position");
					if (propPositionNode.IsValid() && propPositionNode.IsObject())
					{
						props.Position.x = propPositionNode.GetValue<float>("x").Result;
						props.Position.y = propPositionNode.GetValue<float>("y").Result;
						props.Position.z = propPositionNode.GetValue<float>("z").Result;
					}

					JsonNode posVariationNode = pPropsNode.Child("positionvariation");
					if (posVariationNode.IsValid() && posVariationNode.IsObject())
					{
						props.PositionVariation.x = posVariationNode.GetValue<float>("x").Result;
						props.PositionVariation.y = posVariationNode.GetValue<float>("y").Result;
						props.PositionVariation.z = posVariationNode.GetValue<float>("z").Result;
					}

					JsonNode velocityNode = pPropsNode.Child("velocity");
					if (velocityNode.IsValid() && velocityNode.IsObject())
					{
						props.Velocity.x = velocityNode.GetValue<float>("x").Result;
						props.Velocity.y = velocityNode.GetValue<float>("y").Result;
						props.Velocity.z = velocityNode.GetValue<float>("z").Result;
					}

					JsonNode velVariationNode = pPropsNode.Child("velocityvariation");
					if (velVariationNode.IsValid() && velVariationNode.IsObject())
					{
						props.VelocityVariation.x = velVariationNode.GetValue<float>("x").Result;
						props.VelocityVariation.y = velVariationNode.GetValue<float>("y").Result;
						props.VelocityVariation.z = velVariationNode.GetValue<float>("z").Result;
					}

					JsonNode colorBeginNode = pPropsNode.Child("colorbegin");
					if (colorBeginNode.IsValid() && colorBeginNode.IsObject())
					{
						props.ColorBegin.x = colorBeginNode.GetValue<float>("r").Result;
						props.ColorBegin.y = colorBeginNode.GetValue<float>("g").Result;
						props.ColorBegin.z = colorBeginNode.GetValue<float>("b").Result;
						props.ColorBegin.w = colorBeginNode.GetValue<float>("a").Result;
					}

					JsonNode colorEndNode = pPropsNode.Child("colorend");
					if (colorEndNode.IsValid() && colorEndNode.IsObject())
					{
						props.ColorEnd.x = colorEndNode.GetValue<float>("r").Result;
						props.ColorEnd.y = colorEndNode.GetValue<float>("g").Result;
						props.ColorEnd.z = colorEndNode.GetValue<float>("b").Result;
						props.ColorEnd.w = colorEndNode.GetValue<float>("a").Result;
					}

					m_partSystem->GetEmitterArray()[i]->SetEmisionProperties(props);
				}
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