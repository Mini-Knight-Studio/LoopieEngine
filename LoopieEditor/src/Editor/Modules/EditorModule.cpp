#include "EditorModule.h"

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
#include "Loopie/ParticleSystemEn/Emitter.h"
#include "Loopie/Core/Time.h"

#include <glad/glad.h>

namespace Loopie
{
	void EditorModule::OnLoad()
	{
		AssetRegistry::Initialize();

		ScriptingManager::Init();
		Log::Info("Scripting created successfully.");

		Application::GetInstance().GetWindow().SetResizable(true);

		/////SCENE
		Application::GetInstance().CreateScene(""); /// Maybe default One
		m_currentScene = &Application::GetInstance().GetScene();

		JsonData data = Json::ReadFromFile(Application::GetInstance().m_activeProject.GetConfigPath());
		JsonResult<std::string> result = data.Child("last_scene").GetValue<std::string>();
		if (!result.Found || !m_currentScene->ReadAndLoadSceneFile(result.Result))
		{
			CreateCity();
			m_currentScene->CreateEntity({ 0,1,-10 }, { 1,0,0,0 }, { 1,1,1 }, nullptr, "MainCamera")->AddComponent<Camera>();

			//To check that particle system works:
		}
		

		Metadata& metadata = AssetRegistry::GetOrCreateMetadata("assets/materials/outlineMaterial.mat");
		m_selectedObjectMaterial = ResourceManager::GetMaterial(metadata);
		m_selectedObjectMaterial->SetIfEditable(false);
		m_selectedObjectShader = new Shader("assets/shaders/SelectionOutline.shader");
		m_selectedObjectMaterial->SetShader(*m_selectedObjectShader);

		////

		m_assetsExplorer.Init();
		m_topBar.Init();
		m_hierarchy.Init();
		m_inspector.Init();
		m_console.Init();
		m_game.Init();
		m_scene.Init();
		m_mainMenu.Init();
		m_textEditor.Init();

		m_hierarchy.SetScene(m_currentScene);

		Application::GetInstance().m_notifier.AddObserver(this);

	}

	void EditorModule::OnUnload()
	{
		AssetRegistry::Shutdown();
		Application::GetInstance().m_notifier.RemoveObserver(this);
	}

	void EditorModule::OnUpdate()
	{

		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		m_hierarchy.Update(inputEvent);
		m_assetsExplorer.Update(inputEvent);
		m_textEditor.Update(inputEvent);
		m_scene.Update(inputEvent);
		m_topBar.Update(inputEvent);


		//Launch Firework
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

		//TEMPORAL: For updating mainly particles
		for (const auto& [uuid, entity] : Application::GetInstance().GetScene().GetAllEntities())
		{
			if (!entity->HasComponent<ParticleComponent>())
				continue;
			for (auto& component : entity->GetComponents<ParticleComponent>())
			{
				component->Update();
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
			Renderer::SetViewport(0, 0, buffer->GetWidth(), buffer->GetWidth());
			buffer->Bind();
			RenderWorld(cam);
			RenderParticles(cam);
			Renderer::EndScene();

			if (buffer) {
				buffer->Unbind();
			}
		}

		/// SceneWindowRender		
		if (m_scene.IsVisible()) {
			m_scene.StartScene();
			Renderer::BeginScene(m_scene.GetCamera()->GetViewMatrix(), m_scene.GetCamera()->GetProjectionMatrix(), true);
			RenderWorld(m_scene.GetCamera());
			RenderParticles(m_scene.GetCamera());
			Renderer::EndScene();
			m_scene.EndScene();
		}		

		/// GameWindowRender
		if (m_game.IsVisible()) {
			m_game.StartScene();
			if (m_game.GetCamera() && m_game.GetCamera()->GetIsActive()) {
				Renderer::BeginScene(m_game.GetCamera()->GetViewMatrix(), m_game.GetCamera()->GetProjectionMatrix(), false);
				RenderWorld(m_game.GetCamera());
				RenderParticles(m_game.GetCamera());
				Renderer::EndScene();
			}
			m_game.EndScene();
		}
		m_currentScene->FlushRemovedEntities();
	}

	void EditorModule::OnInterfaceRender()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		const float menuBarHeight = ImGui::GetFrameHeight();
		const float topBarHeight = m_topBar.GetToolbarHeight();

		ImVec2 dockPos = viewport->Pos;
		ImVec2 dockSize = viewport->Size;

		dockPos.y += menuBarHeight + topBarHeight;
		dockSize.y -= menuBarHeight + topBarHeight;

		ImGui::SetNextWindowPos(dockPos);
		ImGui::SetNextWindowSize(dockSize);

		ImGuiWindowFlags dockFlags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::Begin("DockSpace", nullptr, dockFlags);

		ImGuiID dockspaceID = ImGui::GetID("EditorDockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

		ImGui::End();

		m_mainMenu.Render();
		m_topBar.Render();
		m_inspector.Render();
		m_console.Render();
		m_hierarchy.Render();
		m_assetsExplorer.Render();
		m_game.Render();
		m_scene.Render();
		m_textEditor.Render();
	}

	void EditorModule::RenderWorld(Camera* camera)
	{	
		Renderer::EnableStencil();
		Renderer::EnableDepth();
		Renderer::Clear();

		// POST
		std::unordered_set<std::shared_ptr<Entity>> entities;
		m_currentScene->GetOctree().CollectVisibleEntitiesFrustum(camera->GetFrustum(), entities);

		std::vector<MeshRenderer*> renderers;
		renderers.reserve(1);

		auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();
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
					if(renderer->GetMesh())
						renderers.push_back(renderer);
				}

				if (Renderer::IsGizmoActive()) {
					if(component->GetTypeID() != Camera::GetTypeIDStatic())
						component->RenderGizmo();
				}
			}

			for (size_t i = 0; i < renderers.size(); i++)
			{
				MeshRenderer* renderer = renderers[i];

				if (!Renderer::IsGizmoActive() || entity != selectedEntity) {
					Renderer::AddRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform());
				}
				else {
					Renderer::SetStencilFunc(Renderer::StencilFunc::ALWAYS, 1, 0xFF);
					Renderer::SetStencilOp(Renderer::StencilOp::KEEP, Renderer::StencilOp::KEEP, Renderer::StencilOp::REPLACE);
					Renderer::SetStencilMask(0xFF);

					Renderer::FlushRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform());

					Renderer::SetStencilFunc(Renderer::StencilFunc::NOTEQUAL, 1, 0xFF);
					Renderer::SetStencilMask(0x00);

					Renderer::FlushRenderItem(renderer->GetMesh()->GetVAO(), m_selectedObjectMaterial, entity->GetTransform());

					Renderer::SetStencilMask(0xFF);
					Renderer::EnableDepth();
					Renderer::DisableStencil();
				}
			}
		}
		Renderer::DisableStencil();
		if (Renderer::IsGizmoActive()) {
			if (selectedEntity)
			{
				Camera* cam = selectedEntity->GetComponent<Camera>();
				if(cam)
					cam->RenderGizmo();
			}
			m_currentScene->GetOctree().DebugDraw(Color::GREEN);
		}
	}
	void EditorModule::RenderParticles(Camera* cam)
	{
		Renderer::DisableStencil();
		Renderer::EnableDepth();
		Renderer::EnableDepthMask();
		Renderer::EnableBlend();
		Renderer::BlendFunction();

		auto& particleEntities = m_currentScene->GetAllEntities();
		for (const auto& [id, entity] : particleEntities)
		{
			const std::vector<Component*>& components = entity->GetComponents();
			for (size_t i = 0; i < components.size(); i++)
			{
				Component* component = components[i];
				if (!component->GetIsActive())
					continue;
				if (component->GetTypeID() == ParticleComponent::GetTypeIDStatic())
				{
					ParticleComponent* particleSystem = static_cast<ParticleComponent*>(component);
					particleSystem->Render(cam);
				}
			}

		}
		Renderer::EnableDepthMask();
		Renderer::DisableBlend();
	}

	void EditorModule::CreateBakerHouse()
	{
		m_scene.ChargeModel("assets/models/BakerHouse.fbx");
		m_scene.ChargeTexture("assets/textures/Baker_house.png");
	}

	void EditorModule::CreateCity()
	{
		m_scene.ChargeModel("assets/models/Street environment_V01.fbx");
		//m_scene.ChargeTexture("assets/textures/Baker_house.png");
	}

	/*void EditorModule::MousePick(Camera* camera)
	{
		Ray ray = Ray{ vec3(0), vec3(1) };
		float distance = -1;
		for (auto& [uuid, entity] : scene->GetAllEntities()) {
			if (!entity->GetIsActive())
				continue;
			MeshRenderer* renderer = entity->GetComponent<MeshRenderer>();
			if (!renderer || !renderer->GetIsActive() || !renderer->GetMesh())
				continue;

			if (!camera->GetFrustum().Intersects(renderer->GetWorldAABB()))
				continue;

			const AABB& worldAABB = renderer->GetWorldAABB();
			vec3 hitPoint;
			if (!worldAABB.IntersectsRay(ray.StartPoint(), ray.EndPoint(), hitPoint)) {
				continue;
			}

			const MeshData& data = renderer->GetMesh()->GetData();
			for (int i = 0; i < data.IndicesAmount/3; i++)
			{
				std::vector<vec3> vertices;
				Triangle t;
				if (!renderer->GetTriangle(i, t))continue;
				vertices[0] = t.v0;
				vertices[1] = t.v1;
				vertices[2] = t.v2;
				vec3 hitPoint;
				if (ray.Intersects(vertices, true, hitPoint))
				{
					if (distance == -1 || distance > (hitPoint - ray.StartPoint()).length())continue;
					HierarchyInterface::s_SelectedEntity = entity;
					distance = (hitPoint - ray.StartPoint()).length();
				}
			}
		}
	}*/

	void EditorModule::OnNotify(const EngineNotification& type)
	{
		if (type == EngineNotification::OnProjectChange) {
			AssetRegistry::Initialize();
			Application::GetInstance().GetWindow().SetTitle(Application::GetInstance().m_activeProject.GetProjectName().c_str());
			m_assetsExplorer.Reload();
			///LOAD SCENE
		}
	}

}