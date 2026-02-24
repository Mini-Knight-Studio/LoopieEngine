#pragma once
#include <vector>
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Components/Component.h"
#include "Loopie/Events/EventTypes.h"


namespace Loopie
{   
	class Emitter;
	class ParticleSystem;
	class Camera;
	
	class ParticleComponent :public Component, public IObserver<TransformNotification>
	{
		
	private:
		
		ParticleSystem* m_partSystem;
	public:
		DEFINE_TYPE(ParticleComponent)
		ParticleComponent();
		ParticleComponent(ParticleSystem* pSystem);
		void Save();
		void Load();
		void Init() override; 
		void Update() override;
		void Render(Camera* cam);
		void Reset();

		void OnNotify(const TransformNotification& id) override;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

		std::vector<Emitter*> GetEmittersVector();
		void AddElemToEmitterVector(Emitter* emitter);

		ParticleSystem* GetParticleSystem();
		void SetParticleSystem(ParticleSystem* pSystem);

	};
}
