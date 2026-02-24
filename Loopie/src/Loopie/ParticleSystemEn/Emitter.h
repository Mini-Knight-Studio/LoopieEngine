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
			bool m_active;

			std::vector<ParticleModule> m_particlePool;
			unsigned int m_poolIndex;

			ParticleProps m_particleProperties;
			std::shared_ptr<Billboard> m_billboard;

			void ParticlePresets(ParticleType type);

		public:
			
			Emitter(unsigned int maxParticles, ParticleType type, BillboardType bType, vec3 position, unsigned int spawnRate);

			void OnUpdate(float dt);
			void OnRender(std::shared_ptr<VertexArray> quadVAO, std::shared_ptr<Material> material, Camera* cam);
			void Emit(const ParticleProps& particleProps);
			
			std::string GetName()const;
			void SetName(std::string n);

			unsigned int GetSpawnrate()const;
			void SetSpawnRate(unsigned int spawnR);

			unsigned int GetmaxParticles()const;
			void SetmaxParticles(unsigned int maxPart);

			vec3 GetPosition() const;
			void SetPosition(const vec3& pos);

			ParticleProps& GetEmissionProperties();
			void SetEmisionProperties(const ParticleProps& partProps);
			
			int GetActiveParticles() const;
			bool IsActive() const;


	};
}

