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
		return particleObj;
	}

	void ParticleComponent::Deserialize(const JsonNode& data)
	{
		
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