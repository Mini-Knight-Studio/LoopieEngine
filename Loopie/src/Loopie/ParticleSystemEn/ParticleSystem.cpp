#pragma once
#include "ParticleSystem.h"
#include "Emitter.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/MaterialImporter.h"

#include "Loopie/Profiler/Profiler.h"

namespace Loopie 
{
	std::shared_ptr<Shader> ParticleSystem::s_ParticleShader = nullptr;
	std::shared_ptr<Material> ParticleSystem::s_ParticleMaterial = nullptr;

	std::shared_ptr<VertexArray> ParticleSystem::s_QuadVAO = nullptr;
	std::shared_ptr<VertexBuffer> ParticleSystem::s_QuadVBO = nullptr;
	std::shared_ptr<IndexBuffer> ParticleSystem::s_QuadIBO = nullptr;

	ParticleSystem::ParticleSystem()
	{
		if (s_ParticleShader == nullptr) {
			s_ParticleShader = std::make_shared<Shader>("assets/shaders/ParticleShader.shader");

			if (s_ParticleShader->GetProgramID() != 0)
			{
				InitializeQuad();
				InitializeMaterial();
			}	
			else
			{
				Log::Error("Particle Shader not set!");
			}
		}


		m_emittersArray.reserve(3);
	}
	ParticleSystem::~ParticleSystem()
	{
		m_emittersArray.clear();
	}
	

	void ParticleSystem::InitializeQuad() 
	{
		float vertices[] =
		{
			-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,    
			 0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,     1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,     0.0f, 1.0f,
		};

		unsigned int indices[] =
		{
			0, 1, 2,
			2, 3, 0
		};

		s_QuadVBO = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
		s_QuadIBO = std::make_shared<IndexBuffer>(indices, 6);

		BufferLayout layout;
		layout.AddLayoutElement(0, GLVariableType::FLOAT, 3, "Position");
		layout.AddLayoutElement(1, GLVariableType::FLOAT, 2, "TexCoord");
		s_QuadVBO->SetLayout(layout);

		s_QuadVAO = std::make_shared<VertexArray>();
		s_QuadVAO->AddBuffer(s_QuadVBO.get(), s_QuadIBO.get());
	}
	void ParticleSystem::InitializeMaterial() 
	{
		Metadata& metadata = AssetRegistry::GetOrCreateMetadata("assets/materials/ParticleMaterial.mat");
		if (!metadata.HasCache) 
		{
			MaterialImporter::ImportMaterial("assets/materials/ParticleMaterial.mat", metadata);
		}
		s_ParticleMaterial = ResourceManager::GetMaterial(metadata);
		s_ParticleMaterial->Load();
		s_ParticleMaterial->SetShader(*s_ParticleShader.get());

		if (!s_ParticleMaterial)
		{
			Log::Error("Failed to load particle material!");
		}
		
	}

	void ParticleSystem::OnUpdate(Transform* transform, float dt, bool active)
	{
		
		if (!m_emittersArray.empty())
		{
		
			vec3 pos = transform->GetPosition();
			quaternion rot = transform->GetRotation();
			vec3 scale = transform->GetWorldScale();

			for (const auto& emitter : m_emittersArray)
			{
				if (emitter)
				{

					if (emitter->GetIsFollowingOwner()) {
						vec3 rotatedOffset = rot * emitter->GetPositionOffSet();
						emitter->SetPosition(pos + rotatedOffset);
					}
					emitter->SetEmitterRotation(rot);
					emitter->SetEmitterScale(scale);

					emitter->OnUpdate(dt, active);
				}
			}
		}
		else
		{ 
			Log::Info("emitter array empty"); 
		}
		
	}
	void ParticleSystem::OnRender(Camera* cam)
	{
		LP_FUNC();
		if (!s_QuadVAO || !s_ParticleMaterial)
		{
			Log::Error("ParticleSystem missing material or quad");
			return;
		}

		
		for (const auto& emitter : m_emittersArray)
		{
			if (emitter)
			{
				emitter->OnRender(s_QuadVAO, s_ParticleMaterial,cam);
			}
		}
	}
	int ParticleSystem::GetActiveParticles() const 
	{
		int total = 0;
		for (const auto& emitter : m_emittersArray)
		{
			if (emitter)
			{
				total += emitter->GetActiveParticles();
			}
		}
		return total;
	}
	const std::vector<std::shared_ptr<Emitter>>& ParticleSystem::GetEmitterArray()const
	{
		return m_emittersArray;
	}
	std::shared_ptr<Emitter> ParticleSystem::GetEmitterByName(const std::string& emitterName) const
	{
		for (const auto& emitter : m_emittersArray)
		{
			if (emitter->GetName() == emitterName)
			{
				return emitter;
			}
		}
		return nullptr;
	}

	int ParticleSystem::GetEmitterIndexByName(const std::string& emitterName) const
	{
		int index = 0;
		for (const auto& emitter : m_emittersArray)
		{
			if (emitter->GetName() == emitterName)
			{
				return index;
			}
			index += 1;
		}
		return -1;
	}

	void ParticleSystem::AddElemToEmitterArray(const std::shared_ptr<Emitter>& em)
	{
		m_emittersArray.push_back(em);
	}
	void ParticleSystem::DeleteElemFromEmitterArray(const std::shared_ptr<Emitter>& em)
	{
		auto it = std::find(m_emittersArray.begin(), m_emittersArray.end(), em);
		if (it != m_emittersArray.end())
		{
			m_emittersArray.erase(it);
		}
	}

	void ParticleSystem::ClearEmitterArray()
	{
		m_emittersArray.clear();
	}
	
}