#pragma once
#include "ParticleModule.h"
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/Resources/Types/Texture.h"
#include "Loopie/ParticleSystemEn/BillBoardComponent.h"

#include "Loopie/Math/MathTypes.h"
#include <vector>
#include <memory>

namespace Loopie 
{
	enum ParticleType;
	class Camera;
	struct ParticleProps
	{
		vec3 Position = vec3(0.0f);
		vec3 Velocity = vec3(0.0f);
		vec3 VelocityVariation = vec3(0.0f);
		vec3 PositionVariation = vec3(0.0f);
		vec4 ColorBegin = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		vec4 ColorEnd = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		float SizeBegin = 1.0f;
		float SizeEnd = 0.0f;
		float SizeVariation = 0.0f;
		float LifeTime = 1.0f;
		
	};
	class Emitter
	{
		private:
			std::string m_name;
			unsigned int m_spawnRate;
			unsigned int m_maxParticles;
			float m_emitterTimer;
			vec3 m_position;
			vec3 m_positionOffSet;
			bool m_active;

			std::vector<ParticleModule> m_particlePool;
			unsigned int m_poolIndex;

			ParticleProps m_particleProperties;
			std::shared_ptr<Billboard> m_billboard;
		public:
			
			Emitter(unsigned int maxParticles, BillboardType bType, vec3 position, unsigned int spawnRate, vec3 posOffSet = vec3(0));

			void OnUpdate(float dt);
			void OnRender(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, Camera* cam);
			void Emit(const ParticleProps& particleProps);
			
			std::string GetName()const;
			void SetName(std::string n);

			unsigned int GetSpawnrate()const;
			void SetSpawnRate(unsigned int spawnR);

			unsigned int GetMaxParticles()const;
			void SetMaxParticles(unsigned int maxPart);

			float GetEmitterTimer()const;
			void SetEmitterTimer(float timer);

			vec3 GetPosition() const;
			void SetPosition(const vec3& pos);

			vec3 GetPositionOffSet() const;
			void SetPositionOffSet(const vec3& posOffSet);

			ParticleProps& GetEmissionProperties();
			void SetEmisionProperties(const ParticleProps& partProps);

			unsigned int GetPoolIndex()const;
			void SetPoolIndex(unsigned int poolIndex);
			
			int GetActiveParticles() const;
			bool IsActive() const;
			void SetActive(bool isActive);

			void ToggleActive();


	};
}

