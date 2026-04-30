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
	class Transform;

	class ParticleSystem
	{
		private:
			std::vector <std::shared_ptr<Emitter>> m_emittersArray;

			static std::shared_ptr<VertexArray> s_QuadVAO;
			static std::shared_ptr<VertexBuffer> s_QuadVBO;
			static std::shared_ptr<IndexBuffer> s_QuadIBO;

			static std::shared_ptr<Material> s_ParticleMaterial;
			static std::shared_ptr<Shader> s_ParticleShader;

			void InitializeQuad();
			void InitializeMaterial();
			
		public:
			ParticleSystem();
			~ParticleSystem();
	

			void OnUpdate(Transform* transform, float dt, bool active);
			void OnRender(Camera* cam);

			void AddElemToEmitterArray(const std::shared_ptr<Emitter>& em);
			void DeleteElemFromEmitterArray(const std::shared_ptr<Emitter>& em);
			void ClearEmitterArray();

			const std::vector<std::shared_ptr<Emitter>>& GetEmitterArray() const;
			std::shared_ptr<Emitter> GetEmitterByName(const std::string& emitterName) const;
			int GetEmitterIndexByName(const std::string& emitterName) const;
			std::shared_ptr<VertexArray> GetQuadVAO() const;
			std::shared_ptr<Material> GetMaterial() const;	
			int GetActiveParticles() const;	


	};
}