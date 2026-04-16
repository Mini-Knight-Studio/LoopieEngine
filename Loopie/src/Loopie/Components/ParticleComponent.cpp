#include "ParticleComponent.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/ParticleSystemEn/ParticleSystem.h"
#include "Loopie/ParticleSystemEn/Emitter.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"

#include "Loopie/Profiler/Profiler.h"

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

	void ParticleComponent::Play()
	{
		m_playing = true;
	}

	void ParticleComponent::Stop()
	{
		m_playing = false;
	}	

	void ParticleComponent::OnUpdate() {
		if (!m_playing)
			return;

		Transform* transform = GetTransform();
		vec3 pos = transform->GetPosition();
		quaternion rot = transform->GetRotation();

		const auto& emitters = m_partSystem.GetEmitterArray();
		for (auto& emitter : emitters) {
			if (emitter->GetParticlesFollowEmitter()) {
				vec3 rotatedOffset = rot * emitter->GetPositionOffSet();
				emitter->SetPosition(pos + rotatedOffset);
			}
			emitter->SetEmitterRotation(rot);
		}
		m_partSystem.OnUpdate((float)Time::GetDeltaTime());
	}

	void ParticleComponent::Render(Camera* cam)
	{
		LP_FUNC();
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
		particleObj.CreateField("playing", IsPlaying());
		for (size_t i = 0; i < m_partSystem.GetEmitterArray().size(); i++)
		{
			JsonNode emitterNode = particleObj.CreateObjectField(std::to_string(i));
			emitterNode.CreateField("name", m_partSystem.GetEmitterArray()[i]->GetName());
			emitterNode.CreateField("spawnrate", m_partSystem.GetEmitterArray()[i]->GetSpawnrate());
			emitterNode.CreateField("maxparticles", m_partSystem.GetEmitterArray()[i]->GetMaxParticles());
			emitterNode.CreateField("emmitertimer", m_partSystem.GetEmitterArray()[i]->GetEmitterTimer());
			emitterNode.CreateField("active", m_partSystem.GetEmitterArray()[i]->GetIsActive());
			emitterNode.CreateField("poolindex", m_partSystem.GetEmitterArray()[i]->GetPoolIndex());
			emitterNode.CreateField("particlefollowemitter", m_partSystem.GetEmitterArray()[i]->GetParticlesFollowEmitter());
			emitterNode.CreateField("localvelocity", m_partSystem.GetEmitterArray()[i]->GetLocalVelocity());
			if(m_partSystem.GetEmitterArray()[i]->GetSprite())
				emitterNode.CreateField("sprite_uuid", m_partSystem.GetEmitterArray()[i]->GetSprite()->GetUUID().Get());

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

			JsonNode rotNode = emitterNode.CreateObjectField("rotation");
			rotNode.CreateField("x", m_partSystem.GetEmitterArray()[i]->GetEmitterRotation().x);
			rotNode.CreateField("y", m_partSystem.GetEmitterArray()[i]->GetEmitterRotation().y);
			rotNode.CreateField("z", m_partSystem.GetEmitterArray()[i]->GetEmitterRotation().z);
			rotNode.CreateField("w", m_partSystem.GetEmitterArray()[i]->GetEmitterRotation().w);

		}
		
		return particleObj;
	}

	void ParticleComponent::Deserialize(const JsonNode& data)
	{

		int emmittersCount = data.GetValue<int>("emitterscount", 0).Result;
		m_playing = data.GetValue<bool>("playing", true).Result;


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
				m_partSystem.GetEmitterArray()[i]->SetParticlesFollowEmitter(node.GetValue<bool>("particlefollowemitter", false).Result);
				m_partSystem.GetEmitterArray()[i]->SetLocalVelocity(node.GetValue<bool>("localvelocity", false).Result);

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

					JsonNode rotNode = node.Child("rotation");
					if (rotNode.IsValid() && rotNode.IsObject())
					{
						glm::quat rot;
						rot.x = rotNode.GetValue<float>("x").Result;
						rot.y = rotNode.GetValue<float>("y").Result;
						rot.z = rotNode.GetValue<float>("z").Result;
						rot.w = rotNode.GetValue<float>("w").Result;

						m_partSystem.GetEmitterArray()[i]->SetEmitterRotation(rot);
					}

					UUID spriteID = UUID(node.GetValue<std::string>("sprite_uuid").Result);
					if (UUID::IsValid(spriteID.Get()))
					{
						UUID id = UUID(data.GetValue<std::string>("sprite_uuid").Result);
						Metadata* meta = AssetRegistry::GetMetadata(id);
						if (meta)
							m_partSystem.GetEmitterArray()[i]->SetSprite(ResourceManager::GetTexture(*meta));
					}

					m_partSystem.GetEmitterArray()[i]->SetEmisionProperties(props);
				}
			}
		}
	}
	void ParticleComponent::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const ParticleComponent& otherParticle = static_cast<const ParticleComponent&>(other);

		m_partSystem = otherParticle.m_partSystem;
		m_partSystem.ClearEmitterArray();

		const auto& otherEmitters = otherParticle.m_partSystem.GetEmitterArray();

		for (const auto& otherEmitter : otherEmitters)
		{
			if (!otherEmitter)
				continue;


			auto newEmitter = std::make_shared<Emitter>(
				otherEmitter->GetMaxParticles(),
				CAMERA_FACING,
				otherEmitter->GetPosition(),
				otherEmitter->GetSpawnrate(),
				otherEmitter->GetPositionOffSet()
			);

			newEmitter->SetName(otherEmitter->GetName());
			newEmitter->SetSpawnRate(otherEmitter->GetSpawnrate());
			newEmitter->SetMaxParticles(otherEmitter->GetMaxParticles());
			newEmitter->SetEmitterTimer(otherEmitter->GetEmitterTimer());
			newEmitter->SetActive(otherEmitter->GetIsActive());
			newEmitter->SetPoolIndex(otherEmitter->GetPoolIndex());
			newEmitter->SetParticlesFollowEmitter(otherEmitter->GetParticlesFollowEmitter());
			newEmitter->SetLocalVelocity(otherEmitter->GetLocalVelocity());
			newEmitter->SetEmitterRotation(otherEmitter->GetEmitterRotation());

			newEmitter->SetEmisionProperties(otherEmitter->GetEmissionProperties());

			newEmitter->SetSprite(otherEmitter->GetSprite());
			m_partSystem.AddElemToEmitterArray(newEmitter);
		}
	}
	const std::vector<std::shared_ptr<Emitter>>& ParticleComponent::GetEmittersVector()
	{
		return m_partSystem.GetEmitterArray();
	}
	std::shared_ptr<Emitter> ParticleComponent::GetEmitterByName(const std::string& emitterName)
	{
		return m_partSystem.GetEmitterByName(emitterName);
	}

	int ParticleComponent::GetEmitterIndexByName(const std::string& emitterName)
	{
		return m_partSystem.GetEmitterIndexByName(emitterName);
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