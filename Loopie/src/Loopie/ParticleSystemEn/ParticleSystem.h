#pragma once
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/Render/Shader.h"
#include <vector>
#include <memory>

namespace Loopie
{
	class Emitter;
	class Camera;

	class ParticleSystem
	{
		private:
			std::vector <std::shared_ptr<Emitter>> m_emittersArray;

			std::shared_ptr<VertexArray> m_quadVAO;
			std::shared_ptr<VertexBuffer> m_quadVBO;
			std::shared_ptr<IndexBuffer> m_quadIBO;
			std::shared_ptr<Material> m_particleMaterial;
			Shader m_particleShader = Shader("assets/shaders/ParticleShader.shader");;

			void InitializeQuad();
			void InitializeMaterial();
			
		public:
			ParticleSystem();
			~ParticleSystem();
	

			void OnUpdate(float dt);
			void OnRender(Camera* cam);

			void AddElemToEmitterArray(const std::shared_ptr<Emitter>& em);
			void DeleteElemFromEmitterArray(const std::shared_ptr<Emitter>& em);

			const std::vector<std::shared_ptr<Emitter>>& GetEmitterArray() const;
			std::shared_ptr<VertexArray> GetQuadVAO() const;
			std::shared_ptr<Material> GetMaterial() const;	
			int GetActiveParticles() const;	
	};
}