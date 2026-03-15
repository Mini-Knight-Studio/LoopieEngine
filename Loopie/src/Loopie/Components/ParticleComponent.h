#pragma once
#include <vector>
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Components/Component.h"
#include "Loopie/ParticleSystemEn/ParticleModule.h"
#include "Loopie/ParticleSystemEn/Emitter.h"
#include "Loopie/ParticleSystemEn/ParticleSystem.h"
#include "Loopie/Events/EventTypes.h"


namespace Loopie
{   
	class Emitter;
	class Camera;
	
	class ParticleComponent :public Component, public IObserver<TransformNotification>
	{
		
	private:
		ParticleSystem m_partSystem;
	public:
		DEFINE_TYPE(ParticleComponent)
		ParticleComponent();
		void Save();
		void Load();
		void Init() override; 
		void Update() override;
		void Render(Camera* cam);
		void Reset();

		void OnNotify(const TransformNotification& id) override;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

		const std::vector<std::shared_ptr<Emitter>>& GetEmittersVector();
		void AddElemToEmitterVector(const std::shared_ptr<Emitter>& emitter);

		ParticleSystem& GetParticleSystem();
	};
}
