#include "EditorModule.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Project/ProjectConfig.h"

//// Test
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Render/UIRenderer.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Render/Colors.h"

#include "Loopie/Math/MathTypes.h"

#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Collisions/CollisionProcessor.h"
#include "Loopie/Audio/AudioManager.h"

#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/TextureImporter.h"
#include "Loopie/Math/Ray.h"
#include "Loopie/Importers/MaterialImporter.h"
#include "Loopie/Resources/Types/Material.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Animator.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/AudioListener.h"
#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Components/Button.h"
#include "Loopie/Components/UIManager.h"
#include "Loopie/Components/CanvasScaler.h"
#include "Loopie/Components/Canvas.h"


#include "Loopie/ParticleSystemEn/ParticleSystem.h"  
#include "Loopie/Components/ParticleComponent.h"
#include "Loopie/ParticleSystemEn/Emitter.h"
#include "Loopie/Core/Time.h"

///


#include "Loopie/Profiler/Profiler.h"


#include <memory>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <glad/glad.h>

namespace Loopie
{
	void EditorModule::CollectOverlayUIJobsRecursive(const std::shared_ptr<Entity>& entity, const vec2& overlayScale,
		int canvasSortingLayer, int canvasOrderInLayer,
		std::vector<UIJob>& outJobs, uint64_t& inOutTraversal)
	{
		if (!entity || !entity->GetIsActive())
			return;

		RectTransform* rt = entity->GetComponent<RectTransform>();
		if (rt)
		{
			if (Image* img = entity->GetComponent<Image>(); img && img->GetIsActive())
			{
				outJobs.push_back(UIJob{ entity, UIJobType::Image, overlayScale,
					canvasSortingLayer, canvasOrderInLayer,
					img->GetSortingLayer(), img->GetOrderInLayer(),
					inOutTraversal++ });
			}

			if (Text* text = entity->GetComponent<Text>(); text && text->GetIsActive())
			{
				outJobs.push_back(UIJob{ entity, UIJobType::Text, overlayScale,
					canvasSortingLayer, canvasOrderInLayer,
					text->GetSortingLayer(), text->GetOrderInLayer(),
					inOutTraversal++ });
			}
		}

		for (const auto& child : entity->GetChildren())
			CollectOverlayUIJobsRecursive(child, overlayScale, canvasSortingLayer, canvasOrderInLayer, outJobs, inOutTraversal);
	}

	void EditorModule::DrawOverlayUIJob(const UIJob& job)
	{
		const auto& entity = job.Entity;
		if (!entity || !entity->GetIsActive())
			return;

		RectTransform* rt = entity->GetComponent<RectTransform>();
		if (!rt)
			return;

		switch (job.Type)
		{
			case UIJobType::Image:
			{
				Image* img = entity->GetComponent<Image>();
				if (!img || !img->GetIsActive())
					return;

				const vec3 p = rt->GetWorldPosition();
				const vec3 ws3 = rt->GetWorldScale();
				const vec2 ws(ws3.x, ws3.y);
				const vec3 bmin = rt->GetLocalBoundsMin();
				const vec3 bmax = rt->GetLocalBoundsMax();
				const vec2 s(bmax.x - bmin.x, bmax.y - bmin.y);

				const vec2 pixelSize(s.x * ws.x * job.OverlayScale.x, s.y * ws.y * job.OverlayScale.y);
				const vec2 pixelPos((p.x + bmin.x * ws.x) * job.OverlayScale.x, (p.y + bmin.y * ws.y) * job.OverlayScale.y);

				vec4 color = img->GetTint();
				std::shared_ptr<Texture> texture = img->GetTexture();

				if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
				{
					button->GetCurrentColor(color);
					button->GetCurrentTexture(texture);
				}

				UIRenderer::DrawImage(pixelPos, pixelSize, texture, color, img->GetUVRect());
				break;
			}

			case UIJobType::Text:
			{
				Text* text = entity->GetComponent<Text>();
				if (!text || !text->GetIsActive())
					return;

				const vec3 p = rt->GetWorldPosition();
				const vec3 ws3 = rt->GetWorldScale();
				const vec2 ws(ws3.x, ws3.y);
				const vec3 bmin = rt->GetLocalBoundsMin();
				const vec3 bmax = rt->GetLocalBoundsMax();
				const vec2 s(bmax.x - bmin.x, bmax.y - bmin.y);

				const vec2 pixelPos((p.x + bmin.x * ws.x) * job.OverlayScale.x, (p.y + bmin.y * ws.y) * job.OverlayScale.y);
				const vec2 pixelSize(s.x * ws.x * job.OverlayScale.x, s.y * ws.y * job.OverlayScale.y);

				vec4 textColor = text->GetColor();
				if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
					button->GetCurrentColor(textColor);

				UIRenderer::DrawTextContainer(pixelPos, pixelSize, text->GetText(), text->GetFont(), textColor, text->GetScale() * job.OverlayScale.x,
					text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment(), text->GetWrapMode(),
					text->GetLineSpacing(), text->GetWordSpacing(), text->GetLetterSpacing());
				break;
			}
		}
	}

	void EditorModule::CollectWorldUIJobsRecursive(const std::shared_ptr<Entity>& entity,
		int canvasSortingLayer, int canvasOrderInLayer,
		std::vector<UIJob>& outJobs, uint64_t& inOutTraversal)
	{
		if (!entity || !entity->GetIsActive())
			return;

		RectTransform* rt = entity->GetComponent<RectTransform>();
		if (rt)
		{
			if (Image* img = entity->GetComponent<Image>(); img && img->GetIsActive())
			{
				outJobs.push_back(UIJob{ entity, UIJobType::Image, vec2(1.0f),
					canvasSortingLayer, canvasOrderInLayer,
					img->GetSortingLayer(), img->GetOrderInLayer(),
					inOutTraversal++ });
			}

			if (Text* text = entity->GetComponent<Text>(); text && text->GetIsActive())
			{
				outJobs.push_back(UIJob{ entity, UIJobType::Text, vec2(1.0f),
					canvasSortingLayer, canvasOrderInLayer,
					text->GetSortingLayer(), text->GetOrderInLayer(),
					inOutTraversal++ });
			}
		}

		for (const auto& child : entity->GetChildren())
			CollectWorldUIJobsRecursive(child, canvasSortingLayer, canvasOrderInLayer, outJobs, inOutTraversal);
	}

	void EditorModule::DrawWorldUIJob(const UIJob& job)
	{
		const auto& entity = job.Entity;
		if (!entity || !entity->GetIsActive())
			return;

		RectTransform* rt = entity->GetComponent<RectTransform>();
		if (!rt)
			return;

		switch (job.Type)
		{
			case UIJobType::Image:
			{
				Image* img = entity->GetComponent<Image>();
				if (!img || !img->GetIsActive())
					return;

				const std::shared_ptr<Texture> tex = img->GetTexture();
				if (!tex)
					return;

				const vec3 bmin = rt->GetLocalBoundsMin();
				const vec3 bmax = rt->GetLocalBoundsMax();
				const float w = bmax.x - bmin.x;
				const float h = bmax.y - bmin.y;

				matrix4 model = rt->GetLocalToWorldMatrix()
					* glm::translate(matrix4(1.0f), vec3(bmin.x, bmin.y, 0.0f))
					* glm::scale(matrix4(1.0f), vec3(w, h, 1.0f));

				vec4 color = img->GetTint();
				std::shared_ptr<Texture> texture = img->GetTexture();

				if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
				{
					button->GetCurrentColor(color);
					button->GetCurrentTexture(texture);
				}

				UIRenderer::DrawImageWorld(model, texture, color, img->GetUVRect());
				break;
			}

			case UIJobType::Text:
			{
				Text* text = entity->GetComponent<Text>();
				if (!text || !text->GetIsActive())
					return;

				const vec3 bmin = rt->GetLocalBoundsMin();
				const vec3 bmax = rt->GetLocalBoundsMax();
				const float w = bmax.x - bmin.x;
				const float h = bmax.y - bmin.y;

				const matrix4 model = rt->GetLocalToWorldMatrix() * glm::translate(matrix4(1.0f), vec3(bmin.x, bmin.y, 0.0f));

				UIRenderer::DrawTextWorld(model, vec2(w, h), text->GetText(), text->GetFont(), text->GetColor(), text->GetScale(),
					text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment(), text->GetWrapMode(),
					text->GetLineSpacing(), text->GetWordSpacing(), text->GetLetterSpacing());
				break;
			}
		}
	}

	void EditorModule::OnLoad()
	{
		LP_FUNC();

		AssetRegistry::Initialize();


		ScriptingContext& context = ScriptingManager::GetContext();

		std::string projectDir = Application::GetInstance().m_activeProject.GetGameDLLPath().string();
		context.CoreAssemblyFilepath = "../Loopie/Loopie.Core.dll";
		context.AppAssemblyFilepath = projectDir;
		context.CompilerAssemblyFilepath = "../Loopie/Loopie.ScriptCompiler.dll";
		context.ExternalAssemblyPath = "../Loopie/";
		context.EnableRecompile = true;

		ScriptingManager::Init();
		Log::Info("Scripting created successfully.");

		Application::GetInstance().GetWindow().SetResizable(true);

		/////SCENE
		Application::GetInstance().CreateScene(); /// Just the scene object
		m_currentScene = &Application::GetInstance().GetScene();

		JsonData data = ProjectConfig::GetData();
		JsonResult result = data.Child("last_scene").GetValue<std::string>();
		std::filesystem::path absolutePath = Application::GetInstance().m_activeProject.GetProjectPath().parent_path() / (result.Result);
		if (!result.Found || !m_currentScene->ReadAndLoadSceneFile(absolutePath.string()))
		{
			m_currentScene->CreateEntity({ 0,1,-10 }, { 1,0,0,0 }, { 1,1,1 }, nullptr, "MainCamera")->AddComponent<Camera>();

			//To check that particle system works:
		}
		

		Metadata& metadata = AssetRegistry::GetOrCreateMetadata("assets\\materials\\outlineMaterial.mat");
		m_selectedObjectMaterial = ResourceManager::GetMaterial(metadata);
		m_selectedObjectMaterial->SetIfEditable(false);
		m_selectedObjectShader = new Shader("assets\\shaders\\SelectionOutline.shader");
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
		//m_textEditor.Init();

		m_hierarchy.SetScene(m_currentScene);

		Application::GetInstance().m_notifier.AddObserver(this);

	}

	void EditorModule::OnUnload()
	{
		LP_FUNC();

		UIRenderer::Shutdown();
		AssetRegistry::Shutdown();
		Application::GetInstance().m_notifier.RemoveObserver(this);
	}

	void EditorModule::OnUpdate()
	{
		LP_FUNC();

		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();
		{
			LP_SCOPE_N("Audio Update");
			AudioManager::Update();
		}

		//// Update Components
		DebugGameMode mode = m_topBar.GetCurrentMode();
		{
			LP_SCOPE_N("Update Components");
			if (!UpdateComponents(mode)) {
				m_topBar.SetMode(DebugGameMode::END);
			}
		}
		{
			LP_SCOPE_N("Collision");
			CollisionProcessor::Process();
		}
		//// 

		{
			LP_SCOPE_N("Octree Update");
			LooseOctree& octree = m_currentScene->GetOctree();

			for (const auto& [uuid, entity] : m_currentScene->GetAllEntities())
			{
				if (entity->GetTransform()->HasChangedThisFrame())
				{
					octree.Update(entity);
					entity->GetTransform()->CleanChangesFlag();
				}
			}
		}

		{
			LP_SCOPE_N("Editor UI Updates");
			m_hierarchy.Update(inputEvent);
			m_assetsExplorer.Update(inputEvent);
			//m_textEditor.Update(inputEvent);
			m_scene.Update(inputEvent);
			m_topBar.Update(inputEvent);
			m_mainMenu.Update(inputEvent);
		}

		{
			LP_SCOPE_N("Camera Setup");
			bool visibleEditorCam = m_scene.IsVisible();
			if (visibleEditorCam) {
				m_scene.PrepareFrameBuffer();
				m_scene.GetCamera()->SetRenderTarget(visibleEditorCam ? m_scene.GetFrameBuffer() : nullptr);
			}

			visibleEditorCam = m_game.IsVisible() && m_game.GetCamera() && m_game.GetCamera()->GetIsActive();
			if (visibleEditorCam) {
				m_game.PrepareFrameBuffer();
				m_game.GetCamera()->SetRenderTarget(visibleEditorCam ? m_game.GetFrameBuffer() : nullptr);
			}

		}

		{
			LP_SCOPE_N("Rendering Loop");

			const std::vector<Camera*>& cameras = Renderer::GetRendererCameras();
			for (Camera* cam : cameras)
			{
				if (!cam->GetIsActive())
					continue;

				std::shared_ptr<FrameBuffer> buffer = cam->GetRenderTarget();
				if (!buffer)
					continue;

				{
					LP_SCOPE_N("Shadow Pass");
					RenderShadows(cam);
				}

				{
					LP_SCOPE_N("Main Render");
					buffer->Bind();
					Renderer::SetViewport(0, 0, buffer->GetWidth(), buffer->GetHeight());

					Renderer::BeginScene(cam->GetViewMatrix(), cam->GetProjectionMatrix(), cam->GetIsEditorCamera());

					RenderWorld(cam);
					

					if (cam == m_game.GetCamera() && m_game.ShowGizmos())
					{
						{
							LP_SCOPE_N("Gizmos");

							auto& entities = m_currentScene->GetAllEntities();

							for (const auto& [id, entity] : entities)
							{
								if (!entity->GetIsActive())
									continue;

								auto* components = entity->GetComponentsRaw<ScriptClass>();
								if (!components)
									continue;

								for (auto* component : *components)
								{
									if (!component->GetIsActive())
										continue;

									ScriptClass* script = static_cast<ScriptClass*>(component);
									if (!script->IsValid())
										continue;
									script->InvokeOnDrawGizmo();
								}
							}
						}
					}
					Renderer::SetSceneDepthTexture(buffer->GetDepthId());
					Renderer::SetSceneFrustrumValues(cam->GetNearPlane(), cam->GetFarPlane()); // needed for depth testing

					Renderer::EndScene();
					RenderParticles(cam);

					
					buffer->Unbind();
				}
			}
		}

		{
			LP_SCOPE_N("UI Render");
			if (m_game.IsVisible() && m_game.GetCamera() && m_game.GetCamera()->GetIsActive())
			{
				RenderUI();
			}
		}

		{
			LP_SCOPE_N("Overlay Input");
			ProcessOverlayButtonsInput();
		}

		{
			LP_SCOPE_N("Scene Cleanup");
			m_currentScene->FlushRemovedEntities();
		}
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
		//m_textEditor.Render();
	}

	bool EditorModule::UpdateComponents(DebugGameMode mode)
	{
		LP_FUNC();

		if (mode == DebugGameMode::START) {

			bool errors = false;
			for (const auto& [uuid, entity] : m_currentScene->GetAllEntities()) {
				if (auto* scripts = entity->GetComponentsRaw<ScriptClass>())
				{
					for (Component* comp : *scripts)
					{
						ScriptClass* script = static_cast<ScriptClass*>(comp);
						if (!script->GetScriptingClass()) {
							errors = true;
							Log::Error("Failed to start the game. Check the scripts || Entity: {0}   Component UUID: {1}", script->GetOwner()->GetName(), script->GetUUID().Get());
						}
					}
				}
			}

			if (errors)
			{
				return false;
			}

			ScriptingManager::RuntimeStart();
			m_currentScene->SaveScene("recoverScene.scene");
		}

		if (!ScriptingManager::IsRunning())
			return true;

		if(mode == Loopie::UPDATING || mode== Loopie::NEXTFRAME)
			ScriptingManager::UpdateCoroutines();

		{
			LP_SCOPE_N("Components Update Loops");

			for (const auto& [uuid, entity] : m_currentScene->GetAllEntities()) {
				if (!entity->GetIsActive())
					continue;

				for (auto& component : entity->GetComponentsRaw())
				{
					if (!component->GetLocalIsActive())
						continue;

					component->OnUpdate();

					if (component->GetTypeID() == ScriptClass::GetTypeIDStatic())
					{
						{
							ScriptClass* script = static_cast<ScriptClass*>(component.get());
							LP_SCOPE_N("Script Update");
							if (!script->IsValid())
								continue;
							switch (mode)
							{
							case Loopie::UPDATING:
							case Loopie::NEXTFRAME:
								{
									LP_SCOPE_N("Update");
									script->InvokeOnUpdate();
									if (m_currentScene->HasLoadRequest())
										break;
									break;
								}
							default:
								break;
							}
						}
					}
				}
				if (m_currentScene->HasLoadRequest()) {
					m_currentScene->ReadAndLoadSceneFile(m_currentScene->GetRequestedSceneID());
					break;
				}
			}
		}

		if (mode == DebugGameMode::END || Application::GetInstance().HasToStopScripting()) {
			Application::GetInstance().StopScripting(false);	
			ScriptingManager::RuntimeStop();
			Application::GetInstance().GetScene().ReadAndLoadSceneFile("recoverScene.scene", false);
		}

		return true;
	}

	void EditorModule::RenderWorld(Camera* camera)
	{
		LP_FUNC();

		Renderer::EnableStencil();
		Renderer::EnableDepth();
		Renderer::Clear();

		// POST
		std::unordered_set<Entity*> entities;

		{
			LP_SCOPE_N("Frustum Culling");
			m_currentScene->GetOctree().CollectVisibleEntitiesFrustum(camera->GetFrustum(), entities);
		}

		auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();

		{
			LP_SCOPE_N("Render Submission");
			for (const auto& entity : entities)
			{
				if (!entity->GetIsActive())
					continue;

				if (Renderer::IsGizmoActive() && HierarchyInterface::s_SelectedEntity.lock().get() == entity) {
					
					const auto& components = entity->GetComponentsRaw();
					for (const auto& component : components)
					{

						component->RenderGizmo();
					}
				}

				auto* renderers = entity->GetComponentsRaw<MeshRenderer>();
				if (!renderers)
					continue;

				for (auto* component : *renderers)
				{
					if (!component->GetIsActive())
						continue;

					MeshRenderer* renderer = static_cast<MeshRenderer*>(component);
					if (!renderer->GetMesh())
						continue;

					const MeshData& data = renderer->GetMesh()->GetData();

					std::vector<matrix4> bones = {};
					if (data.HasBones) {
						Animator* animator = renderer->GetLinkedAnimator();
						if (animator) {
							bones = animator->GetRendererData(renderer->GetUUID())->FinalBoneMatrices;
						}
					}

					if (!Renderer::IsGizmoActive() || entity != selectedEntity.get()) {
						Renderer::AddRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform(), bones);
					}
					else {
						Renderer::SetStencilFunc(Renderer::StencilFunc::ALWAYS, 1, 0xFF);
						Renderer::SetStencilOp(Renderer::StencilOp::KEEP, Renderer::StencilOp::KEEP, Renderer::StencilOp::REPLACE);
						Renderer::SetStencilMask(0xFF);

						Renderer::FlushRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform(), bones);

						Renderer::SetStencilFunc(Renderer::StencilFunc::NOTEQUAL, 1, 0xFF);
						Renderer::SetStencilMask(0x00);

						Renderer::FlushRenderItem(renderer->GetMesh()->GetVAO(), m_selectedObjectMaterial, entity->GetTransform());

						Renderer::SetStencilMask(0xFF);
						Renderer::EnableDepth();
						Renderer::DisableStencil();
					}
				}
			}
		}

		RenderSceneUI(camera);

		Renderer::DisableStencil();
		if (Renderer::IsGizmoActive()) {
			if (selectedEntity)
			{
				for (auto& component : selectedEntity->GetComponentsRaw())
					component->RenderGizmo();
			}
			m_currentScene->GetOctree().DebugDraw(Color::MAGENTA);
			CollisionProcessor::RenderGizmos();
		}
	}

	void EditorModule::RenderShadows(const Camera* cam)
	{
		LP_FUNC();

		Renderer::AssignShadowSlots(cam->GetViewProjectionMatrix(), cam->GetProjection(), m_currentScene->GetEntitySpanningBounds());
		for (int i = 0; i < Renderer::GetShadowCastingLightCount(); ++i)
		{
			Frustum frustum;
			frustum.FromMatrix(Renderer::GetShadowSlotMatrix(i));

			std::unordered_set<Entity*> allEntities;
			std::unordered_set<Entity*> dynamicEntities;
			m_currentScene->GetOctree().CollectVisibleEntitiesFrustum(frustum, allEntities);
			SeparateDynamicEntities(allEntities, dynamicEntities);

			std::unordered_set<Entity*> staticEntities = m_currentScene->GetStaticEntities();

			LP_SCOPE_N("Shadow Casters");
			// Static pass, only when dirty
			if (Renderer::BeginStaticShadowPass(i))
			{
				RenderEntityShadows(staticEntities);
				Renderer::EndStaticShadowPass(i);
			}

			// Dynamic pass, every frame
			if (Renderer::BeginDynamicShadowPass(i))
			{
				RenderEntityShadows(dynamicEntities);
				Renderer::EndDynamicShadowPass(i);
			}
		}
	}

	void EditorModule::RenderEntityShadows(const std::unordered_set<Entity*>& entities)
	{
		for (const auto& entity : entities)
		{
			auto* components = entity->GetComponentsRaw<MeshRenderer>();
			if (!components)
				continue;

			for (auto* component : *components)
			{
				if (!component->GetIsActive())
					continue;

				MeshRenderer* renderer = static_cast<MeshRenderer*>(component);
				if (!renderer->GetCastsShadows() || !renderer->GetMesh())
					continue;
				const MeshData& data = renderer->GetMesh()->GetData();

				std::vector<matrix4> bones = {};
				if (data.HasBones) {
					Animator* animator = renderer->GetLinkedAnimator();
					if (animator) {
						bones = animator->GetRendererData(renderer->GetUUID())->FinalBoneMatrices;
					}
				}

				Renderer::FlushShadowItem(renderer->GetMesh()->GetVAO(), entity->GetTransform(), bones);
			}
		}
	}

	void EditorModule::RenderParticles(Camera* cam)
	{
		LP_FUNC();

		Renderer::DisableStencil();
		Renderer::EnableDepth();
		Renderer::SetDepthFunc(Renderer::DepthFunc::LEQUAL);
		Renderer::DisableDepthMask();
		Renderer::EnableBlend();
		Renderer::BlendFunction(Renderer::BlendFactorMode::ONE, Renderer::BlendFactorMode::ONE_MINUS_SRC_ALPHA);

		auto& particleEntities = m_currentScene->GetAllEntities();

		for (const auto& [id, entity] : particleEntities)
		{
			if (!entity->GetIsActive())
				continue;

			auto* components = entity->GetComponentsRaw<ParticleComponent>();
			if (!components)
				continue;

			for (auto* c : *components)
			{
				if (!c->GetIsActive())
					continue;

				static_cast<ParticleComponent*>(c)->Render(cam);
			}
		}

		Renderer::FlushParticles();
		Renderer::EnableDepthMask();
		Renderer::DisableBlend();

		
	}

	void EditorModule::SeparateDynamicEntities(const std::unordered_set<Entity*>& entities, std::unordered_set<Entity*>& dynamicEntities)
	{
		for (const auto& entity : entities)
		{
			if (!entity->GetIsActive())
				continue;
			if (!entity->GetIsStatic())
				dynamicEntities.insert(entity);
		}
	}


	void EditorModule::RenderUIRecursive(const std::shared_ptr<Entity>& entity, vec2& scale)
	{
		if (!entity || !entity->GetIsActive())
			return;

		Image* img = entity->GetComponent<Image>();
		Text* text = entity->GetComponent<Text>();
		RectTransform* rt = entity->GetComponent<RectTransform>();

		if (img && img->GetIsActive() && rt)
		{
			const vec3 p = rt->GetWorldPosition();
			const vec3 ws3 = rt->GetWorldScale();
			const vec2 ws(ws3.x, ws3.y);
			const vec3 bmin = rt->GetLocalBoundsMin();
			const vec3 bmax = rt->GetLocalBoundsMax();
			const vec2 s(bmax.x - bmin.x, bmax.y - bmin.y);

			const vec2 pixelSize(s.x * ws.x * scale.x, s.y * ws.y * scale.y);
			const vec2 pixelPos((p.x + bmin.x * ws.x) * scale.x, (p.y + bmin.y * ws.y) * scale.y);

			vec4 color = img->GetTint();
			std::shared_ptr<Texture> texture = img->GetTexture();

			if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
			{
				button->GetCurrentColor(color);
				button->GetCurrentTexture(texture);
			}

			UIRenderer::DrawImage(pixelPos, pixelSize, texture, color, img->GetUVRect());
		}

		if (text && text->GetIsActive() && rt)
		{
			const vec3 p = rt->GetWorldPosition();
			const vec3 ws3 = rt->GetWorldScale();
			const vec2 ws(ws3.x, ws3.y);
			const vec3 bmin = rt->GetLocalBoundsMin();
			const vec3 bmax = rt->GetLocalBoundsMax();
			const vec2 s(bmax.x - bmin.x, bmax.y - bmin.y);

			const vec2 pixelPos((p.x + bmin.x * ws.x) * scale.x, (p.y + bmin.y * ws.y) * scale.y);
			const vec2 pixelSize(s.x * ws.x * scale.x, s.y * ws.y * scale.y);

			vec4 textColor = text->GetColor();
			if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
				button->GetCurrentColor(textColor);

			UIRenderer::DrawTextContainer(pixelPos, pixelSize, text->GetText(), text->GetFont(), textColor, text->GetScale() * scale.x,
				text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment(), text->GetWrapMode(),
				text->GetLineSpacing(), text->GetWordSpacing(), text->GetLetterSpacing());
		}

		for (const auto& child : entity->GetChildren())
			RenderUIRecursive(child, scale);
	}

	void EditorModule::RenderUI()
	{
		LP_FUNC();
		std::shared_ptr<FrameBuffer> buffer = m_game.GetFrameBuffer();
		if (!buffer)
			return;

		buffer->Bind();

		const float w = static_cast<float>(m_game.GetFrameBuffer()->GetWidth());
		const float h = static_cast<float>(m_game.GetFrameBuffer()->GetHeight());

		if (w <= 0.0f || h <= 0.0f)
		{
			buffer->Unbind();
			return;
		}

		Renderer::SetViewport(0, 0, static_cast<unsigned int>(w), static_cast<unsigned int>(h));

		Renderer::DisableDepth();
		Renderer::DisableStencil();
		Renderer::DisableCulling();

		Renderer::EnableBlend();

		const matrix4 uiView(1.0f);
		const matrix4 uiProj = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);

		Renderer::BeginScene(uiView, uiProj, false);

		std::vector<UIJob> jobs;
		jobs.reserve(256);
		uint64_t traversal = 0;

		for (const auto& [uuid, entity] : m_currentScene->GetAllEntities())
		{
			if (!entity || !entity->GetIsActive())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			if (!canvas || !canvas->GetIsActive())
				continue;

			if (canvas->GetRenderMode() != CanvasRenderMode::ScreenSpaceOverlay)
				continue;

			RectTransform* canvasRt = entity->GetComponent<RectTransform>();
			if (!canvasRt)
				continue;

			const vec2 targetPixels(w, h);

			vec2 canvasUnits(canvasRt->GetWidth(), canvasRt->GetHeight());
			if (auto* scaler = entity->GetComponent<CanvasScaler>(); scaler && scaler->GetIsActive())
			{
				canvasUnits = scaler->ComputeOverlayCanvasSize(targetPixels);
			}
			else
			{
				canvasUnits = targetPixels;
			}

			canvasRt->SetWidth(canvasUnits.x);
			canvasRt->SetHeight(canvasUnits.y);

			if (canvasUnits.x <= 0.0f || canvasUnits.y <= 0.0f)
				continue;

			const vec2 overlayScale(targetPixels.x / canvasUnits.x, targetPixels.y / canvasUnits.y);
			CollectOverlayUIJobsRecursive(entity, overlayScale, canvas->GetSortingLayer(), canvas->GetOrderInLayer(), jobs, traversal);
		}

		std::sort(jobs.begin(), jobs.end(), [](const UIJob& a, const UIJob& b)
			{
				if (a.CanvasSortingLayer != b.CanvasSortingLayer) return a.CanvasSortingLayer < b.CanvasSortingLayer;
				if (a.CanvasOrderInLayer != b.CanvasOrderInLayer) return a.CanvasOrderInLayer < b.CanvasOrderInLayer;
				if (a.ElementSortingLayer != b.ElementSortingLayer) return a.ElementSortingLayer < b.ElementSortingLayer;
				if (a.ElementOrderInLayer != b.ElementOrderInLayer) return a.ElementOrderInLayer < b.ElementOrderInLayer;
				return a.TraversalIndex < b.TraversalIndex;
			});

		for (const UIJob& job : jobs)
			DrawOverlayUIJob(job);

		Renderer::EndScene();

		Renderer::DisableBlend();
		Renderer::EnableDepth();

		buffer->Unbind();
	}

	void EditorModule::RenderSceneUIRecursive(const std::shared_ptr<Entity>& entity)
	{
		if (!entity || !entity->GetIsActive())
			return;

		Image* img = entity->GetComponent<Image>();
		Text* text = entity->GetComponent<Text>();
		RectTransform* rt = entity->GetComponent<RectTransform>();

		if (img && img->GetIsActive() && rt)
		{
			const std::shared_ptr<Texture> tex = img->GetTexture();
			if (tex)
			{
				const vec3 bmin = rt->GetLocalBoundsMin();
				const vec3 bmax = rt->GetLocalBoundsMax();
				const float w = bmax.x - bmin.x;
				const float h = bmax.y - bmin.y;

				matrix4 model = rt->GetLocalToWorldMatrix()
					* glm::translate(matrix4(1.0f), vec3(bmin.x, bmin.y, 0.0f))
					* glm::scale(matrix4(1.0f), vec3(w, h, 1.0f));
				vec4 color = img->GetTint();
				std::shared_ptr<Texture> texture = img->GetTexture();

				if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
				{
					button->GetCurrentColor(color);
					button->GetCurrentTexture(texture);
				}

				UIRenderer::DrawImageWorld(model, texture, color, img->GetUVRect());
			}
		}

		if (text && text->GetIsActive() && rt)
		{
			const vec3 bmin = rt->GetLocalBoundsMin();
			const vec3 bmax = rt->GetLocalBoundsMax();
			const float w = bmax.x - bmin.x;
			const float h = bmax.y - bmin.y;

			const matrix4 model = rt->GetLocalToWorldMatrix() * glm::translate(matrix4(1.0f), vec3(bmin.x, bmin.y, 0.0f));

			UIRenderer::DrawTextWorld(model, vec2(w, h), text->GetText(), text->GetFont(), text->GetColor(), text->GetScale(),
				text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment(), text->GetWrapMode(),
				text->GetLineSpacing(), text->GetWordSpacing(), text->GetLetterSpacing());
		}

		for (const auto& child : entity->GetChildren())
			RenderSceneUIRecursive(child);
	}

	void EditorModule::RenderSceneUI(Camera* camera)
	{
		LP_FUNC();

		Renderer::EnableBlend();
		Renderer::DisableCulling();
		Renderer::EnableDepth();
		Renderer::SetDepthWrite(true);

		const bool isSceneView = (camera == m_scene.GetCamera());

		std::vector<UIJob> jobs;
		jobs.reserve(256);
		uint64_t traversal = 0;

		for (const auto& [uuid, entity] : m_currentScene->GetAllEntities())
		{
			if (!entity || !entity->GetIsActive())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			if (!canvas || !canvas->GetIsActive())
				continue;

			if (!isSceneView && canvas->GetRenderMode() == CanvasRenderMode::ScreenSpaceOverlay)
				continue;

			RectTransform* canvasRt = entity->GetComponent<RectTransform>();
			if (auto* scaler = entity->GetComponent<CanvasScaler>(); scaler && scaler->GetIsActive())
			{
				if (canvasRt)
				{
					const vec2 sceneTargetPixels((float)m_game.GetGameSize().x, (float)m_game.GetGameSize().y);
					const vec2 canvasUnits = scaler->ComputeOverlayCanvasSize(sceneTargetPixels);
					canvasRt->SetWidth(canvasUnits.x);
					canvasRt->SetHeight(canvasUnits.y);
				}
			}

			CollectWorldUIJobsRecursive(entity, canvas->GetSortingLayer(), canvas->GetOrderInLayer(), jobs, traversal);
		}

		std::sort(jobs.begin(), jobs.end(), [](const UIJob& a, const UIJob& b)
			{
				if (a.CanvasSortingLayer != b.CanvasSortingLayer) return a.CanvasSortingLayer < b.CanvasSortingLayer;
				if (a.CanvasOrderInLayer != b.CanvasOrderInLayer) return a.CanvasOrderInLayer < b.CanvasOrderInLayer;
				if (a.ElementSortingLayer != b.ElementSortingLayer) return a.ElementSortingLayer < b.ElementSortingLayer;
				if (a.ElementOrderInLayer != b.ElementOrderInLayer) return a.ElementOrderInLayer < b.ElementOrderInLayer;
				return a.TraversalIndex < b.TraversalIndex;
			});

		for (const UIJob& job : jobs)
			DrawWorldUIJob(job);

		Renderer::DisableBlend();
	}

	void EditorModule::ProcessOverlayButtonsInput()
	{
		LP_FUNC();

		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		if (!m_currentScene)
			return;

		const bool mouseOverGame = m_game.IsVisible() && m_game.IsMouseOverGame();
		if (!mouseOverGame)
		{
			return;
		}

		const ivec2 gameSize = m_game.GetGameSize();
		if (gameSize.x <= 0 || gameSize.y <= 0)
		{
			return;
		}

		const vec2 mouseLocalPx = m_game.GetMousePosGameLocal();
		const vec2 targetPixels(static_cast<float>(gameSize.x), static_cast<float>(gameSize.y));

		UIManager* uiManager = nullptr;
		for (const auto& [uuid, e] : m_currentScene->GetAllEntities())
		{
			if (!e || !e->GetIsActive())
				continue;
			uiManager = e->GetComponent<UIManager>();
			if (uiManager && uiManager->GetIsActive())
				break;
			uiManager = nullptr;
		}

		const bool justDown = (inputEvent.GetMouseButtonStatus(0) == KeyState::DOWN);
		bool hitAnyOnPress = false;
		bool selectionSetOnPress = false;

		for (const auto& [uuid, entity] : m_currentScene->GetAllEntities())
		{
			if (!entity || !entity->GetIsActive())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			RectTransform* canvasRt = entity->GetComponent<RectTransform>();
			if (!canvas || !canvasRt || !canvas->GetIsActive())
				continue;
			if (canvas->GetRenderMode() != CanvasRenderMode::ScreenSpaceOverlay)
				continue;

			vec2 canvasUnits(canvasRt->GetWidth(), canvasRt->GetHeight());
			if (auto* scaler = entity->GetComponent<CanvasScaler>(); scaler && scaler->GetIsActive())
			{
				canvasUnits = scaler->ComputeOverlayCanvasSize(targetPixels);
			}
			else
			{
				canvasUnits = targetPixels;
			}

			const float cw = canvasUnits.x;
			const float ch = canvasUnits.y;
			if (cw <= 0.0f || ch <= 0.0f)
				continue;

			const float sx = targetPixels.x / cw;
			const float sy = targetPixels.y / ch;

			const vec2 mouseCanvas(mouseLocalPx.x / sx, ch - (mouseLocalPx.y / sy));

			static bool s_pressedInside = false;
			static bool s_releasedInside = false;
			ProcessOverlayButtonsRecursive(entity, mouseCanvas, mouseOverGame, inputEvent, uiManager,
				hitAnyOnPress, selectionSetOnPress,
				s_pressedInside, s_releasedInside);
			if (s_releasedInside) {
				s_pressedInside = false;
				s_releasedInside = false;
			}
		}

		if (justDown && !hitAnyOnPress)
		{
			if (uiManager)
				uiManager->ClearSelection();
		}
	}

	void EditorModule::ProcessOverlayButtonsRecursive(const std::shared_ptr<Loopie::Entity>& entity, const vec2& mouseCanvas, bool mouseOverGame,
		const Loopie::InputEventManager& input, UIManager* uiManager,
		bool& hitAnyOnPress, bool& selectionSetOnPress,
		bool& pressedInsideAny, bool& releasedInsideAny)
	{
		if (!entity || !entity->GetIsActive())
			return;

		Button* button = entity->GetComponent<Button>();
		RectTransform* rt = entity->GetComponent<RectTransform>();

		bool hovered = false;
		if (mouseOverGame && button && rt && button->GetIsActive())
		{
			const vec3 p = rt->GetWorldPosition();
			const vec3 ws3 = rt->GetWorldScale();
			const vec2 ws(ws3.x, ws3.y);
			const vec3 bmin = rt->GetLocalBoundsMin();
			const vec3 bmax = rt->GetLocalBoundsMax();

			const float x0 = p.x + bmin.x * ws.x;
			const float y0 = p.y + bmin.y * ws.y;
			const float x1 = p.x + bmax.x * ws.x;
			const float y1 = p.y + bmax.y * ws.y;

			const float minX = glm::min(x0, x1);
			const float maxX = glm::max(x0, x1);
			const float minY = glm::min(y0, y1);
			const float maxY = glm::max(y0, y1);

			hovered = (mouseCanvas.x >= minX && mouseCanvas.x <= maxX &&
					   mouseCanvas.y >= minY && mouseCanvas.y <= maxY);

			const bool focused = button->IsFocused();
			button->SetHovered(hovered || focused);

			const bool down = (input.GetMouseButtonStatus(0) == KeyState::DOWN) ||
							  (input.GetMouseButtonStatus(0) == KeyState::REPEAT);
			const bool justDown = (input.GetMouseButtonStatus(0) == KeyState::DOWN);
			const bool up = (input.GetMouseButtonStatus(0) == KeyState::UP);

			if (justDown && hovered)
			{
				hitAnyOnPress = true;
				pressedInsideAny = true;
				if (uiManager && !selectionSetOnPress)
				{
					uiManager->SetSelectedEntity(entity->GetUUID());
					selectionSetOnPress = true;
				}
			}


			if (pressedInsideAny || down)
				button->SetPressed(down && (hovered || focused) && pressedInsideAny);

			if (up && pressedInsideAny)
			{
				if (hovered) {
					button->TriggerClick();
					pressedInsideAny = false;
					if (uiManager)
					{
						uiManager->ClearSelection();
						selectionSetOnPress = false;
					}
				}

				releasedInsideAny = true;
				button->SetPressed(false);
			}
		}

		for (const auto& child : entity->GetChildren())
			ProcessOverlayButtonsRecursive(child, mouseCanvas, mouseOverGame, input, uiManager,
				hitAnyOnPress, selectionSetOnPress,
				pressedInsideAny, releasedInsideAny);
	}


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