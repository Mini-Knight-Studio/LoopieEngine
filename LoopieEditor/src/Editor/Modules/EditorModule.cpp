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
#include "Loopie/Components/CanvasScaler.h"

#include <memory>

///

#include <glad/glad.h>

namespace Loopie
{
	void EditorModule::OnLoad()
	{
		AssetRegistry::Initialize();


		ScriptingContext& context = ScriptingManager::GetContext();

		std::string projectDir = Application::GetInstance().m_activeProject.GetGameDLLPath().string();
		context.CoreAssemblyFilepath = "../Loopie/Loopie.Core.dll";
		context.AppAssemblyFilepath = projectDir;
		context.CompilerAssemblyFilepath = "../Loopie/Loopie.ScriptCompiler.dll";
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
		UIRenderer::Shutdown();
		AssetRegistry::Shutdown();
		Application::GetInstance().m_notifier.RemoveObserver(this);
	}

	void EditorModule::OnUpdate()
	{
		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();
		AudioManager::Update();

		//// Update Components
		DebugGameMode mode = m_topBar.GetCurrentMode();
		if (!UpdateComponents(mode)) {
			m_topBar.SetMode(DebugGameMode::END);
		}
		CollisionProcessor::Process();
		//// 

		m_hierarchy.Update(inputEvent);
		m_assetsExplorer.Update(inputEvent);
		m_textEditor.Update(inputEvent);
		m_scene.Update(inputEvent);
		m_topBar.Update(inputEvent);
		m_mainMenu.Update(inputEvent);
		

		/// RenderToTarget
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

			if (buffer)
				buffer->Unbind();
		}
		///

		/// SceneWindowRender		
		if (m_scene.IsVisible()) {
			m_scene.StartScene();
			Renderer::BeginScene(m_scene.GetCamera()->GetViewMatrix(), m_scene.GetCamera()->GetProjectionMatrix(), true);
			RenderWorld(m_scene.GetCamera());
			Renderer::EndScene();

			const float sw = (float)m_scene.GetFrameBuffer()->GetWidth();
			const float sh = (float)m_scene.GetFrameBuffer()->GetHeight();
			const matrix4 uiView(1.0f);
			const matrix4 uiProj = glm::ortho(0.0f, sw, sh, 0.0f, -1.0f, 1.0f);

			m_scene.EndScene();
		}	
		///

		/// GameWindowRender
		if (m_game.IsVisible()) {
			m_game.StartScene();
			if (m_game.GetCamera() && m_game.GetCamera()->GetIsActive()) {
				Renderer::BeginScene(m_game.GetCamera()->GetViewMatrix(), m_game.GetCamera()->GetProjectionMatrix(), false);
				UpdateComponents(Loopie::GIZMO);
				RenderWorld(m_game.GetCamera());
				Renderer::EndScene();

				// UI pass (ortographic overlay)
				RenderUI();
			}
			m_game.EndScene();
		}

		ProcessOverlayButtonsInput();

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

	bool EditorModule::UpdateComponents(DebugGameMode mode)
	{
		if (mode == DebugGameMode::START) {

			bool errors = false;
			for (const auto& [uuid, entity] : m_currentScene->GetAllEntities()) {
				std::vector<ScriptClass*> scripts = entity->GetComponents<ScriptClass>();
				for (size_t i = 0; i < scripts.size(); i++)
				{
					if (!scripts[i]->GetScriptingClass()) {
						errors = true;
						Log::Error("Failed to start the game. Check the scripts || Entity: {0}   Component UUID: {1}", scripts[i]->GetOwner()->GetName(), scripts[i]->GetUUID().Get());
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

		for (const auto& [uuid, entity] : m_currentScene->GetAllEntities()) {
			if (!entity->GetIsActive())
				continue;
			const std::vector<Component*>& components = entity->GetComponents();
			for (size_t i = 0; i < components.size(); i++)
			{
				Component* component = components[i];
				if (!component->GetLocalIsActive())
					continue;
				component->OnUpdate();

				if (component->GetTypeID() == ScriptClass::GetTypeIDStatic())
				{
					ScriptClass* script = static_cast<ScriptClass*>(component);
					switch (mode)
					{
					case Loopie::UPDATING:
					case Loopie::NEXTFRAME:
						script->InvokeOnUpdate();
						if (m_currentScene->HasLoadRequest())
							break;
						break;
					case Loopie::GIZMO:
						script->InvokeOnDrawGizmo();
						break;
					default:
						break;
					}
				}
			}
			if (m_currentScene->HasLoadRequest()) {
				m_currentScene->ReadAndLoadSceneFile(m_currentScene->GetRequestedSceneID());
				break;
			}
		}

		if (mode == DebugGameMode::END) {
			ScriptingManager::RuntimeStop();
			Application::GetInstance().GetScene().ReadAndLoadSceneFile("recoverScene.scene", false);
		}

		return true;
	}

	Canvas* EditorModule::FindCanvasInParents(const std::shared_ptr<Loopie::Entity>& entity)
	{
		auto current = entity;
		while (current)
		{
			if (auto* c = current->GetComponent<Loopie::Canvas>())
				return c;

			current = current->GetParent().lock();
		}
		return nullptr;
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
				if (!component->GetLocalIsActive())
					continue;
				if (component->GetTypeID() == MeshRenderer::GetTypeIDStatic()) {
					MeshRenderer* renderer = static_cast<MeshRenderer*>(component);
					if(renderer->GetMesh())
						renderers.push_back(renderer);
				}

				if (Renderer::IsGizmoActive()) {
					if (component->GetTypeID() != Camera::GetTypeIDStatic()) {
						if (HierarchyInterface::s_SelectedEntity.lock() == entity)
							component->RenderGizmo();
						if (entity->HasComponent<Canvas>())
							component->RenderGizmo();
					}
				}
			}

			for (size_t i = 0; i < renderers.size(); i++)
			{
				MeshRenderer* renderer = renderers[i];
				const MeshData& data = renderer->GetMesh()->GetData();

				std::vector<matrix4> bones = {};
				if (data.HasBones) {
					Animator* animator = renderer->GetLinkedAnimator();
					if (animator) {
						bones = animator->GetRendererData(renderer->GetUUID()).FinalBoneMatrices;
					}
				}

				if (!Renderer::IsGizmoActive() || entity != selectedEntity) {
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

		RenderSceneUI(camera);

		Renderer::DisableStencil();
		if (Renderer::IsGizmoActive()) {
			if (selectedEntity)
			{
				Camera* cam = selectedEntity->GetComponent<Camera>();
				if(cam)
					cam->RenderGizmo();
			}
			m_currentScene->GetOctree().DebugDraw(Color::MAGENTA);
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

			const vec2 s(rt->GetWidth(), rt->GetHeight());

			const vec2 pixelSize(s.x * ws.x * scale.x, s.y * ws.y * scale.y);
			const vec2 pixelPos(p.x * scale.x, p.y * scale.y);

			vec4 color = img->GetTint();
			std::shared_ptr<Texture> texture = img->GetTexture();

			if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
			{
				button->GetCurrentColor(color);
				button->GetCurrentTexture(texture);
			}

			UIRenderer::DrawImage(pixelPos, pixelSize, texture, color);
		}

		if (text && text->GetIsActive() && rt)
		{
			const vec3 p = rt->GetWorldPosition();
			const vec3 ws3 = rt->GetWorldScale();
			const vec2 ws(ws3.x, ws3.y);

			const vec2 s(rt->GetWidth(), rt->GetHeight());

			const vec2 pixelPos(p.x * scale.x, p.y * scale.y);
			const vec2 pixelSize(s.x * ws.x * scale.x, s.y * ws.y * scale.y);

			UIRenderer::DrawText(pixelPos, pixelSize, text->GetText(), text->GetFont(), text->GetColor(), text->GetScale(),
				text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment());
		}

		for (const auto& child : entity->GetChildren())
			RenderUIRecursive(child, scale);
	}

	void EditorModule::RenderUI()
	{
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
				if (scaler->GetScaleMode() == CanvasScaleMode::ScaleWithCanvasSize)
				{
					scaler->SetReferenceResolution(vec2(canvasRt->GetWidth(), canvasRt->GetHeight()));
				}
				else if (scaler->GetScaleMode() == CanvasScaleMode::ConstantPixelSize)
				{
					canvasRt->SetWidth(m_game.GetGameSize().x);
					canvasRt->SetHeight(m_game.GetGameSize().y);
				}
			}
			else
			{
				canvasUnits = targetPixels;
			}

			if (canvasUnits.x <= 0.0f || canvasUnits.y <= 0.0f)
				continue;

			RenderUIRecursive(entity, vec2(targetPixels.x / canvasUnits.x, targetPixels.y / canvasUnits.y));
		}

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
				const float w = rt->GetWidth();
				const float h = rt->GetHeight();

				matrix4 model = rt->GetLocalToWorldMatrix() * glm::scale(matrix4(1.0f), vec3(w, h, 1.0f));
				vec4 color = img->GetTint();
				std::shared_ptr<Texture> texture = img->GetTexture();

				if (auto button = entity->GetComponent<Button>(); button && button->GetIsActive())
				{
					button->GetCurrentColor(color);
					button->GetCurrentTexture(texture);
				}

				UIRenderer::DrawImageWorld(model, texture, color);
			}
		}

		if (text && text->GetIsActive() && rt)
		{
			const float w = rt->GetWidth();
			const float h = rt->GetHeight();

			const matrix4 model = rt->GetLocalToWorldMatrix();

			UIRenderer::DrawTextWorld(model, vec2(w, h), text->GetText(), text->GetFont(), text->GetColor(), text->GetScale(),
				text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment());
		}

		for (const auto& child : entity->GetChildren())
			RenderSceneUIRecursive(child);
	}

	void EditorModule::RenderSceneUI(Camera* camera)
	{
		Renderer::EnableBlend();
		Renderer::DisableCulling();
		Renderer::EnableDepth();
		Renderer::SetDepthWrite(true);

		const bool isSceneView = (camera == m_scene.GetCamera());

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
				if (scaler->GetScaleMode() == CanvasScaleMode::ScaleWithCanvasSize)
				{
					scaler->SetReferenceResolution(vec2(canvasRt->GetWidth(), canvasRt->GetHeight()));
				}
				else if (scaler->GetScaleMode() == CanvasScaleMode::ConstantPixelSize)
				{
					canvasRt->SetWidth(m_game.GetGameSize().x);
					canvasRt->SetHeight(m_game.GetGameSize().y);
				}
			}

			RenderSceneUIRecursive(entity);
		}

		Renderer::DisableBlend();
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

	void EditorModule::ProcessOverlayButtonsInput()
	{
		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

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
			ProcessOverlayButtonsRecursive(entity, mouseCanvas, mouseOverGame, inputEvent, s_pressedInside);
		}
	}

	void EditorModule::ProcessOverlayButtonsRecursive(const std::shared_ptr<Loopie::Entity>& entity, const vec2& mouseCanvas, bool mouseOverGame, const Loopie::InputEventManager& input, bool& pressedInsideAny)
	{
		if (!entity || !entity->GetIsActive())
			return;

		Button* button = entity->GetComponent<Button>();
		RectTransform* rt = entity->GetComponent<RectTransform>();

		bool hovered = false;
		if (mouseOverGame && button && rt && button->GetIsActive())
		{
			const vec3 p = rt->GetLocalPosition();
			const float x = p.x;
			const float y = p.y;
			const float w = rt->GetWidth();
			const float h = rt->GetHeight();

			hovered = (mouseCanvas.x >= x && mouseCanvas.x <= x + w &&
					   mouseCanvas.y >= y && mouseCanvas.y <= y + h);

			button->SetHovered(hovered);

			const bool down = (input.GetMouseButtonStatus(0) == KeyState::DOWN) ||
							  (input.GetMouseButtonStatus(0) == KeyState::REPEAT);
			const bool justDown = (input.GetMouseButtonStatus(0) == KeyState::DOWN);
			const bool up = (input.GetMouseButtonStatus(0) == KeyState::UP);

			if (justDown && hovered)
			{
				pressedInsideAny = true;
			}

			button->SetPressed(down && hovered && pressedInsideAny);

			if (up && pressedInsideAny)
			{
				if (hovered)
					button->TriggerClick();

				pressedInsideAny = false;
				button->SetPressed(false);
			}
		}

		for (const auto& child : entity->GetChildren())
			ProcessOverlayButtonsRecursive(child, mouseCanvas, mouseOverGame, input, pressedInsideAny);
	}
}