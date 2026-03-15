#pragma once
#include "ParticleSystem.h"
#include "Emitter.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/MaterialImporter.h"

namespace Loopie 
{
	ParticleSystem::ParticleSystem()
	{
		if (m_particleShader.GetProgramID() != 0)
		{
			InitializeQuad();
		    InitializeMaterial();
		}
		else 
		{
			Log::Error("Particle Shader not set!"); 
		}
	}
	ParticleSystem::~ParticleSystem()
	{
		m_emittersArray.clear();
	}
	

	void ParticleSystem::InitializeQuad() 
	{
		float vertices[] =
		{
			-0.5f, -0.5f, 0.0f,    
			 0.5f, -0.5f, 0.0f,   
			 0.5f,  0.5f, 0.0f,   
			-0.5f,  0.5f, 0.0f,    
		};

		unsigned int indices[] =
		{
			0, 1, 2,
			2, 3, 0
		};

		m_quadVBO = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
		m_quadIBO = std::make_shared<IndexBuffer>(indices, 6);

		BufferLayout layout;
		layout.AddLayoutElement(0, GLVariableType::FLOAT, 3, "Position");
		m_quadVBO->SetLayout(layout);

		m_quadVAO = std::make_shared<VertexArray>();
		m_quadVAO->AddBuffer(m_quadVBO.get(), m_quadIBO.get());
	}
	void ParticleSystem::InitializeMaterial() 
	{
		Metadata& metadata = AssetRegistry::GetOrCreateMetadata("assets/materials/ParticleMaterial.mat");
		if (!metadata.HasCache) 
		{
			MaterialImporter::ImportMaterial("assets/materials/ParticleMaterial.mat", metadata);
		}
		m_particleMaterial = ResourceManager::GetMaterial(metadata);
		m_particleMaterial->Load();
		m_particleMaterial->SetShader(m_particleShader);

		if (!m_particleMaterial)
		{
			Log::Error("Failed to load particle material!");
		}
		
	}

	void ParticleSystem::OnUpdate(float dt)
	{
		if (!m_emittersArray.empty())
		{
			
			for (const auto emitter : m_emittersArray)
			{
				if (emitter)
				{
					emitter->OnUpdate(dt);
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
		if (!m_quadVAO || !m_particleMaterial)
		{
			Log::Error("ParticleSystem missing material or quad");
			return;
		}

		
		for (const auto emitter : m_emittersArray)
		{
			if (emitter)
			{
				emitter->OnRender(m_quadVAO, m_particleMaterial,cam);
			}
		}
	}
	int ParticleSystem::GetActiveParticles() const 
	{
		int total = 0;
		for (const auto emitter : m_emittersArray)
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
	
}