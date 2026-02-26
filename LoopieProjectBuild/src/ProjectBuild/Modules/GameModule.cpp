#include "GameModule.h"

#include "Loopie/Core/Application.h"

//// Test
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Render/Colors.h"

#include "Loopie/Math/MathTypes.h"

#include "Loopie/Scripting/ScriptingManager.h"

#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/TextureImporter.h"
#include "Loopie/Math/Ray.h"
#include "Loopie/Importers/MaterialImporter.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Resources/Types/Material.h"
///

#include <glad/glad.h>

namespace Loopie
{
	void GameModule::OnLoad()
	{
		std::filesystem::path gamePath = "Game";	
		Application::GetInstance().m_activeProject.Open(std::filesystem::absolute(gamePath));

		AssetRegistry::Initialize();

		ScriptingManager::Init();
		Log::Info("Scripting created successfully.");

		Application::GetInstance().GetWindow().SetResizable(true);

		/////SCENE
		Application::GetInstance().CreateScene(""); /// Maybe default One
		m_currentScene = &Application::GetInstance().GetScene();

		JsonData data = Json::ReadFromFile(Application::GetInstance().m_activeProject.GetConfigPath());
		JsonResult<std::string> result = data.Child("last_scene").GetValue<std::string>();
		m_currentScene->ReadAndLoadSceneFile(result.Result);

		////
		AssetRegistry::RefreshAssetRegistry();

		m_game.Init();

		Application::GetInstance().m_notifier.AddObserver(this);

	}

	void GameModule::OnUnload()
	{
		AssetRegistry::Shutdown();
		Application::GetInstance().m_notifier.RemoveObserver(this);
	}

	void GameModule::OnUpdate()
	{

		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		m_game.Update(inputEvent);

		const std::vector<Camera*>& cameras = Renderer::GetRendererCameras();
		for (const auto cam : cameras)
		{
			std::shared_ptr<FrameBuffer> buffer = cam->GetRenderTarget();
			if (buffer)
			{
				buffer->Bind();
				buffer->Clear();
				buffer->Unbind();
			}
		}

		/// RenderToTarget
		for (const auto cam : cameras)
		{
			if (!cam->GetIsActive())
				continue;

			std::shared_ptr<FrameBuffer> buffer = cam->GetRenderTarget();
			if (!buffer)
				continue;

			Renderer::BeginScene(cam->GetViewMatrix(), cam->GetProjectionMatrix(), false);
			Renderer::SetViewport(0, 0, buffer->GetWidth(), buffer->GetHeight());
			buffer->Bind();
			RenderWorld(cam);
			Renderer::EndScene();

			if (buffer) {
				buffer->Unbind();
			}
		}
		ivec2 size = Application::GetInstance().GetWindow().GetSize();
		
		m_game.GetCamera()->SetViewport(0, 0, size.x, size.y);
		Renderer::SetViewport(0, 0, size.x, size.y);

			if (m_game.GetCamera() && m_game.GetCamera()->GetIsActive()) {
				Renderer::BeginScene(m_game.GetCamera()->GetViewMatrix(), m_game.GetCamera()->GetProjectionMatrix(), false);
				RenderWorld(m_game.GetCamera());
				Renderer::EndScene();
			}

		m_currentScene->FlushRemovedEntities();
	}

	void GameModule::OnInterfaceRender()
	{
		m_game.Render();
	}

	void GameModule::RenderWorld(Camera* camera)
	{
		Renderer::EnableStencil();
		Renderer::EnableDepth();
		Renderer::Clear();

		// POST
		std::unordered_set<std::shared_ptr<Entity>> entities;
		m_currentScene->GetOctree().CollectVisibleEntitiesFrustum(camera->GetFrustum(), entities);

		std::vector<MeshRenderer*> renderers;
		renderers.reserve(1);

		for (const auto& entity : entities)
		{
			if (!entity->GetIsActive())
				continue;

			const std::vector<Component*>& components = entity->GetComponents();
			renderers.clear();
			for (size_t i = 0; i < components.size(); i++)
			{
				Component* component = components[i];
				if (!component->GetIsActive())
					continue;
				if (component->GetTypeID() == MeshRenderer::GetTypeIDStatic()) {
					MeshRenderer* renderer = static_cast<MeshRenderer*>(component);
					if (renderer->GetMesh())
						renderers.push_back(renderer);
				}

				if (Renderer::IsGizmoActive()) {
					if (component->GetTypeID() != Camera::GetTypeIDStatic())
						component->RenderGizmo();
				}
			}

			for (size_t i = 0; i < renderers.size(); i++)
			{
				MeshRenderer* renderer = renderers[i];

				if (!Renderer::IsGizmoActive()) {
					Renderer::AddRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform());
				}
			}
		}
		Renderer::DisableStencil();

	}

	void GameModule::OnNotify(const EngineNotification& type)
	{
		
	}

}