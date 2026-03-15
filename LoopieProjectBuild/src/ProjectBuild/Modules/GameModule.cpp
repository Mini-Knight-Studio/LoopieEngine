#include "GameModule.h"

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
#include "Loopie/Resources/Types/Material.h"

#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/TextureImporter.h"
#include "Loopie/Math/Ray.h"
#include "Loopie/Importers/MaterialImporter.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Animator.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/CanvasScaler.h"
#include "Loopie/Components/AudioListener.h"
#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Components/Button.h"
///

#include <glad/glad.h>

namespace Loopie
{
	void GameModule::OnLoad()
	{
		std::filesystem::path gamePath = "Game";	
		Application::GetInstance().m_activeProject.Open(std::filesystem::absolute(gamePath));
		Application::GetInstance().GetWindow().SetTitle(Application::GetInstance().m_activeProject.GetNameFromConfig().c_str());

		AssetRegistry::Initialize();


		ScriptingContext& context = ScriptingManager::GetContext();

		std::string projectDir = Application::GetInstance().m_activeProject.GetGameDLLPath().string();
		context.CoreAssemblyFilepath = "../Loopie/Loopie.Core.dll";
		context.AppAssemblyFilepath = projectDir;
		context.EnableRecompile = false;

		ScriptingManager::Init();
		Log::Info("Scripting created successfully.");

		Application::GetInstance().GetWindow().SetResizable(true);

		/////SCENE
		Application::GetInstance().CreateScene(); /// Just the scene object
		m_currentScene = &Application::GetInstance().GetScene();

		JsonData data = ProjectConfig::GetData();
		JsonResult result = data.Child("build_scene").GetValue<std::string>();
		std::filesystem::path absolutePath = std::filesystem::absolute(result.Result);
		m_currentScene->ReadAndLoadSceneFile(absolutePath.string());

		////
		AssetRegistry::RefreshAssetRegistry();

		Application::GetInstance().m_notifier.AddObserver(this);

		mode = DebugGameMode::START;

	}

	void GameModule::OnUnload()
	{
		UIRenderer::Shutdown();
		AssetRegistry::Shutdown();
		Application::GetInstance().m_notifier.RemoveObserver(this);
	}

	void GameModule::OnUpdate()
	{

		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();
		AudioManager::Update();

		//// Update Components
		if (!UpdateComponents(mode)) {
			mode = DebugGameMode::END;
		}
		CollisionProcessor::Process();
		//// 

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

		/// GameWindowRender

		Camera* cam = Camera::GetMainCamera();
		if (cam) {
			ivec2 size = Application::GetInstance().GetWindow().GetSize();
			cam->SetViewport(0, 0, size.x, size.y);
			Renderer::SetViewport(0, 0, size.x, size.y);

			if (cam && cam->GetIsActive()) {
				Renderer::BeginScene(cam->GetViewMatrix(), cam->GetProjectionMatrix(), false);
				RenderWorld(cam);
				Renderer::EndScene();
				RenderUI();
			}
		}
		
		///

		ProcessOverlayButtonsInput();

		m_currentScene->FlushRemovedEntities();

		if(mode == DebugGameMode::START)
			mode = DebugGameMode::UPDATING;
		if(mode == END)
			mode = DebugGameMode::DEACTIVATED;
	}


	bool GameModule::UpdateComponents(DebugGameMode mode)
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
		}



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
		}

		return true;
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
				if (!component->GetLocalIsActive())
					continue;
				if (component->GetTypeID() == MeshRenderer::GetTypeIDStatic()) {
					MeshRenderer* renderer = static_cast<MeshRenderer*>(component);
					if (renderer->GetMesh())
						renderers.push_back(renderer);
				}
			}

			for (size_t i = 0; i < renderers.size(); i++)
			{
				MeshRenderer* renderer = renderers[i];
				const MeshData& data = renderer->GetMesh()->GetData();

				std::vector<matrix4> bones = {};
				if (data.HasBones) {
					Animator* animator = renderer->GetLinkedAnimator();
					if (animator && animator->HasAnimation()) {
						bones = animator->GetRendererData(renderer->GetUUID()).FinalBoneMatrices;
					}
				}

				Renderer::AddRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform(), bones);
			}
		}

		RenderSceneUI(camera);

		Renderer::DisableStencil();
	}

	void GameModule::RenderUIRecursive(const std::shared_ptr<Entity>& entity, vec2& scale)
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

			UIRenderer::DrawImage(pixelPos, pixelSize, texture, color);
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

			UIRenderer::DrawText(pixelPos, pixelSize, text->GetText(), text->GetFont(), text->GetColor(), text->GetScale(),
				text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment());
		}

		for (const auto& child : entity->GetChildren())
			RenderUIRecursive(child, scale);
	}

	void GameModule::RenderUI()
	{
		ivec2 size = Application::GetInstance().GetWindow().GetSize();
		const float w = static_cast<float>(size.x);
		const float h = static_cast<float>(size.y);

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
					canvasRt->SetWidth(size.x);
					canvasRt->SetHeight(size.y);
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
	}

	void GameModule::RenderSceneUIRecursive(const std::shared_ptr<Entity>& entity)
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

				UIRenderer::DrawImageWorld(model, texture, color);
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
				text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment());
		}

		for (const auto& child : entity->GetChildren())
			RenderSceneUIRecursive(child);
	}

	void GameModule::RenderSceneUI(Camera* camera)
	{
		Renderer::EnableBlend();
		Renderer::DisableCulling();
		Renderer::EnableDepth();
		Renderer::SetDepthWrite(true);

		ivec2 size = Application::GetInstance().GetWindow().GetSize();

		for (const auto& [uuid, entity] : m_currentScene->GetAllEntities())
		{
			if (!entity || !entity->GetIsActive())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			if (!canvas || !canvas->GetIsActive())
				continue;

			if (canvas->GetRenderMode() == CanvasRenderMode::ScreenSpaceOverlay)
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
					canvasRt->SetWidth(size.x);
					canvasRt->SetHeight(size.y);
				}
			}

			RenderSceneUIRecursive(entity);
		}

		Renderer::DisableBlend();
	}


	void GameModule::ProcessOverlayButtonsInput()
	{
		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		const vec2 mouseLocalPx = inputEvent.GetMousePosition();

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

			const float cw = canvasRt->GetWidth();
			const float ch = canvasRt->GetHeight();
			if (cw <= 0.0f || ch <= 0.0f)
				continue;


			ivec2 size = Application::GetInstance().GetWindow().GetSize();
			const float w = static_cast<float>(size.x);
			const float h = static_cast<float>(size.y);

			const float sx = static_cast<float>(w) / cw;
			const float sy = static_cast<float>(h) / ch;

			const vec2 mouseCanvas(mouseLocalPx.x / sx, ch - (mouseLocalPx.y / sy));

			static bool s_pressedInside = false;
			ProcessOverlayButtonsRecursive(entity, mouseCanvas, true, inputEvent, s_pressedInside);
		}
	}

	void GameModule::ProcessOverlayButtonsRecursive(const std::shared_ptr<Loopie::Entity>& entity, const vec2& mouseCanvas, bool mouseOverGame, const Loopie::InputEventManager& input, bool& pressedInsideAny)
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