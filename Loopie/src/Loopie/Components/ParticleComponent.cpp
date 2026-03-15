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
	void ParticleComponent::OnUpdate() 
	{
		vec3 pos = GetTransform()->GetPosition();
		vec3 localPos = GetTransform()->GetLocalPosition();
		for (size_t i = 0; i < GetEmittersVector().size(); i++)
		{
			GetEmittersVector()[i]->SetPosition(pos + GetEmittersVector()[i]->GetPositionOffSet());
		}
		
		float dt = (float)Time::GetDeltaTime();
		m_partSystem.OnUpdate(dt);
	}
	void ParticleComponent::Render(Camera* cam)
	{
		m_partSystem.OnRender(cam);
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

		particleObj.CreateField("emitterscount", m_partSystem.GetEmitterArray().size());
		for (size_t i = 0; i < m_partSystem.GetEmitterArray().size(); i++)
		{
			JsonNode emitterNode = particleObj.CreateObjectField(std::to_string(i));
			emitterNode.CreateField("name", m_partSystem.GetEmitterArray()[i]->GetName());
			emitterNode.CreateField("spawnrate", m_partSystem.GetEmitterArray()[i]->GetSpawnrate());
			emitterNode.CreateField("maxparticles", m_partSystem.GetEmitterArray()[i]->GetMaxParticles());
			emitterNode.CreateField("emmitertimer", m_partSystem.GetEmitterArray()[i]->GetEmitterTimer());
			emitterNode.CreateField("active", m_partSystem.GetEmitterArray()[i]->IsActive());
			emitterNode.CreateField("poolindex", m_partSystem.GetEmitterArray()[i]->GetPoolIndex());

			JsonNode vectorNode = emitterNode.CreateObjectField("position");
			vectorNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetPosition().x);
			vectorNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetPosition().y);
			vectorNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetPosition().z);

			vectorNode = emitterNode.CreateObjectField("positionoffset");
			vectorNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetPositionOffSet().x);
			vectorNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetPositionOffSet().y);
			vectorNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetPositionOffSet().z);


			JsonNode pProps = emitterNode.CreateObjectField("particleprops");

			pProps.CreateField("sizebegin", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().SizeBegin);
			pProps.CreateField("sizeend", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().SizeEnd);
			pProps.CreateField("sizevariation", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().SizeVariation);
			pProps.CreateField("lifetime", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().LifeTime);

			vectorNode = pProps.CreateObjectField("position");
			vectorNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().Position.x);
			vectorNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().Position.y);
			vectorNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().Position.z);

			vectorNode = pProps.CreateObjectField("positionvariation");
			vectorNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().PositionVariation.x);
			vectorNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().PositionVariation.y);
			vectorNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().PositionVariation.z);

			vectorNode = pProps.CreateObjectField("velocity");
			vectorNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().Velocity.x);
			vectorNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().Velocity.y);
			vectorNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().Velocity.z);

			vectorNode = pProps.CreateObjectField("velocityvariation");
			vectorNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().VelocityVariation.x);
			vectorNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().VelocityVariation.y);
			vectorNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().VelocityVariation.z);

			vectorNode = pProps.CreateObjectField("colorbegin");
			vectorNode.CreateField("r", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.x);
			vectorNode.CreateField("g", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.y);
			vectorNode.CreateField("b", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.z);
			vectorNode.CreateField("a", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorBegin.w);


			vectorNode = pProps.CreateObjectField("colorend");
			vectorNode.CreateField("r", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.x);
			vectorNode.CreateField("g", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.y);
			vectorNode.CreateField("b", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.z);
			vectorNode.CreateField("a", m_partSystem.GetEmitterArray()[i]->GetEmissionProperties().ColorEnd.w);
		}
		
		return particleObj;
	}

	void ParticleComponent::Deserialize(const JsonNode& data)
	{

		int emmittersCount = data.GetValue<int>("emitterscount", 0).Result;
		for (size_t i = 0; i < emmittersCount; i++)
		{
			m_partSystem.AddElemToEmitterArray(std::make_shared<Emitter>(1000, CAMERA_FACING, GetTransform()->GetPosition(), 100));

			JsonNode node = data.Child(std::to_string(i));
			if (node.IsValid() && node.IsObject())
			{
				m_partSystem.GetEmitterArray()[i]->SetName(node.GetValue<string>("name").Result);
				m_partSystem.GetEmitterArray()[i]->SetSpawnRate(node.GetValue<unsigned int>("spawnrate").Result);
				m_partSystem.GetEmitterArray()[i]->SetMaxParticles(node.GetValue<unsigned int>("maxparticles").Result);
				m_partSystem.GetEmitterArray()[i]->SetEmitterTimer(node.GetValue<float>("emmitertimer").Result);
				m_partSystem.GetEmitterArray()[i]->SetActive(node.GetValue<bool>("active",false).Result);
				m_partSystem.GetEmitterArray()[i]->SetPoolIndex(node.GetValue<unsigned int>("poolindex").Result);

				JsonNode positionNode = node.Child("position");
				if (positionNode.IsValid() && positionNode.IsObject())
				{
					glm::vec3 position;
					position.x = positionNode.GetValue<float>("x").Result;
					position.y = positionNode.GetValue<float>("y").Result;
					position.z = positionNode.GetValue<float>("z").Result;
					m_partSystem.GetEmitterArray()[i]->SetPosition(position);
				}

				JsonNode positionOffsetNode = node.Child("positionoffset");
				if (positionOffsetNode.IsValid() && positionOffsetNode.IsObject())
				{
					glm::vec3 positionOffset;
					positionOffset.x = positionOffsetNode.GetValue<float>("x").Result;
					positionOffset.y = positionOffsetNode.GetValue<float>("y").Result;
					positionOffset.z = positionOffsetNode.GetValue<float>("z").Result;
					m_partSystem.GetEmitterArray()[i]->SetPositionOffSet(positionOffset);
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

					m_partSystem.GetEmitterArray()[i]->SetEmisionProperties(props);
				}
			}
		}
	}
	const std::vector<std::shared_ptr<Emitter>>& ParticleComponent::GetEmittersVector()
	{
		return m_partSystem.GetEmitterArray();
	}
	void ParticleComponent::AddElemToEmitterVector(const std::shared_ptr<Emitter>& emitter)
	{
		m_partSystem.AddElemToEmitterArray(emitter);
	}
	ParticleSystem& ParticleComponent::GetParticleSystem()
	{
		return m_partSystem;
	}
}