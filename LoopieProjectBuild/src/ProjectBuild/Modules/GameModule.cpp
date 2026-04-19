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

#include "Loopie/ParticleSystemEn/ParticleSystem.h"  
#include "Loopie/Components/ParticleComponent.h"
#include "Loopie/ParticleSystemEn/Emitter.h"
#include "Loopie/Core/Time.h"
///

#include <memory>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <glad/glad.h>

namespace Loopie
{
	namespace Jobs
	{
		enum class UIJobType
		{
			Image,
			Text,
		};

		struct UIJob
		{
			std::shared_ptr<Entity> Entity;
			UIJobType Type;
			vec2 OverlayScale{ 1.0f, 1.0f };
			int CanvasSortingLayer = 0;
			int CanvasOrderInLayer = 0;
			int ElementSortingLayer = 0;
			int ElementOrderInLayer = 0;
			uint64_t TraversalIndex = 0;
		};

		static bool UIJobLess(const UIJob& a, const UIJob& b)
		{
			if (a.CanvasSortingLayer != b.CanvasSortingLayer) return a.CanvasSortingLayer < b.CanvasSortingLayer;
			if (a.CanvasOrderInLayer != b.CanvasOrderInLayer) return a.CanvasOrderInLayer < b.CanvasOrderInLayer;
			if (a.ElementSortingLayer != b.ElementSortingLayer) return a.ElementSortingLayer < b.ElementSortingLayer;
			if (a.ElementOrderInLayer != b.ElementOrderInLayer) return a.ElementOrderInLayer < b.ElementOrderInLayer;
			return a.TraversalIndex < b.TraversalIndex;
		}

		static void CollectOverlayUIJobsRecursive(const std::shared_ptr<Entity>& entity, const vec2& overlayScale,
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

		static void DrawOverlayUIJob(const UIJob& job)
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

					UIRenderer::DrawText(pixelPos, pixelSize, text->GetText(), text->GetFont(), text->GetColor(), text->GetScale(),
						text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment());
					break;
				}
			}
		}

		static void CollectWorldUIJobsRecursive(const std::shared_ptr<Entity>& entity,
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

		static void DrawWorldUIJob(const UIJob& job)
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
						text->GetSizeMode(), text->GetFontSize(), text->GetHorizontalAlignment(), text->GetVerticalAlignment());
					break;
				}
			}
		}

		struct ButtonJob
		{
			std::shared_ptr<Entity> Entity;
			Button* ButtonComp = nullptr;
			RectTransform* Rect = nullptr;
			vec2 MouseCanvas{ 0.0f, 0.0f };
			int CanvasSortingLayer = 0;
			int CanvasOrderInLayer = 0;
			int ElementSortingLayer = 0;
			int ElementOrderInLayer = 0;
			uint64_t TraversalIndex = 0;
		};

		static bool ButtonJobLess(const ButtonJob& a, const ButtonJob& b)
		{
			if (a.CanvasSortingLayer != b.CanvasSortingLayer) return a.CanvasSortingLayer < b.CanvasSortingLayer;
			if (a.CanvasOrderInLayer != b.CanvasOrderInLayer) return a.CanvasOrderInLayer < b.CanvasOrderInLayer;
			if (a.ElementSortingLayer != b.ElementSortingLayer) return a.ElementSortingLayer < b.ElementSortingLayer;
			if (a.ElementOrderInLayer != b.ElementOrderInLayer) return a.ElementOrderInLayer < b.ElementOrderInLayer;
			return a.TraversalIndex < b.TraversalIndex;
		}

		static bool IsButtonJobHovered(const ButtonJob& job)
		{
			if (!job.Entity || !job.Entity->GetIsActive())
				return false;
			if (!job.ButtonComp || !job.Rect || !job.ButtonComp->GetIsActive())
				return false;

			const vec3 p = job.Rect->GetWorldPosition();
			const vec3 ws3 = job.Rect->GetWorldScale();
			const vec2 ws(ws3.x, ws3.y);
			const vec3 bmin = job.Rect->GetLocalBoundsMin();
			const vec3 bmax = job.Rect->GetLocalBoundsMax();

			const float x0 = p.x + bmin.x * ws.x;
			const float y0 = p.y + bmin.y * ws.y;
			const float x1 = p.x + bmax.x * ws.x;
			const float y1 = p.y + bmax.y * ws.y;

			const float minX = glm::min(x0, x1);
			const float maxX = glm::max(x0, x1);
			const float minY = glm::min(y0, y1);
			const float maxY = glm::max(y0, y1);

			return (job.MouseCanvas.x >= minX && job.MouseCanvas.x <= maxX && job.MouseCanvas.y >= minY && job.MouseCanvas.y <= maxY);
		}

		static void CollectOverlayButtonJobsRecursive(const std::shared_ptr<Entity>& e, const vec2& mouseCanvas,
			int canvasSortingLayer, int canvasOrderInLayer,
			std::vector<ButtonJob>& out, uint64_t& inOutTraversal)
		{
			if (!e || !e->GetIsActive())
				return;

			Button* button = e->GetComponent<Button>();
			RectTransform* rt = e->GetComponent<RectTransform>();
			if (button && button->GetIsActive() && rt)
			{
				int elementSortingLayer = button->GetSortingLayer();
				int elementOrderInLayer = button->GetOrderInLayer();
				if (Image* img = e->GetComponent<Image>(); img && img->GetIsActive())
				{
					elementSortingLayer = img->GetSortingLayer();
					elementOrderInLayer = img->GetOrderInLayer();
				}

				out.push_back(ButtonJob{ e, button, rt, mouseCanvas,
					canvasSortingLayer, canvasOrderInLayer,
					elementSortingLayer, elementOrderInLayer,
					inOutTraversal++ });
			}

			for (const auto& child : e->GetChildren())
				CollectOverlayButtonJobsRecursive(child, mouseCanvas, canvasSortingLayer, canvasOrderInLayer, out, inOutTraversal);
		}
	}

	void GameModule::OnLoad()
	{
		std::filesystem::path gamePath = "Game";	
		Application::GetInstance().m_activeProject.Open(std::filesystem::absolute(gamePath));
		Application::GetInstance().GetWindow().SetTitle(Application::GetInstance().m_activeProject.GetNameFromConfig().c_str());

		AssetRegistry::Initialize();


		ScriptingContext& context = ScriptingManager::GetContext();

		std::string projectDir = Application::GetInstance().m_activeProject.GetGameDLLPath().string();
		context.CoreAssemblyFilepath = "../Loopie/Loopie.Core.dll";
		context.ExternalAssemblyPath = "../Loopie/";
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
		Camera* cam = Camera::GetMainCamera();
		if (cam && cam->GetIsActive())
		{
			Renderer::AssignShadowSlots(cam->GetTransform()->GetWorldPosition());
			for (int i = 0; i < Renderer::GetShadowCastingLightCount(); ++i)
			{
				if (Renderer::BeginShadowPass(i))
				{
					RenderShadows(cam);
					Renderer::EndShadowPass(i);
				}
			}

			const std::vector<Camera*>& cameras = Renderer::GetRendererCameras();
			for (Camera* rtCam : cameras)
			{
				if (!rtCam->GetIsActive())
					continue;
				std::shared_ptr<FrameBuffer> buffer = rtCam->GetRenderTarget();
				if (!buffer)
					continue;

				buffer->Bind();
				Renderer::SetViewport(0, 0, buffer->GetWidth(), buffer->GetHeight());
				Renderer::BeginScene(rtCam->GetViewMatrix(), rtCam->GetProjectionMatrix(), false);
				RenderWorld(rtCam);
				RenderParticles(rtCam);
				Renderer::EndScene();
				buffer->Unbind();
			}

			/// GameWindowRender
			ivec2 size = Application::GetInstance().GetWindow().GetSize();
			cam->SetViewport(0, 0, size.x, size.y);
			Renderer::SetViewport(0, 0, size.x, size.y);
			Renderer::BeginScene(cam->GetViewMatrix(), cam->GetProjectionMatrix(), false);
			RenderWorld(cam);
			RenderParticles(cam);
			Renderer::EndScene();
			RenderUI();
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
		std::unordered_set<Entity*> entities;
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
					if (animator) {
						bones = animator->GetRendererData(renderer->GetUUID())->FinalBoneMatrices;
					}
				}

				Renderer::AddRenderItem(renderer->GetMesh()->GetVAO(), renderer->GetMaterial(), entity->GetTransform(), bones);
			}
		}

		RenderSceneUI(camera);

		Renderer::DisableStencil();
	}

	void GameModule::RenderParticles(Camera* cam) {
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

	void GameModule::RenderShadows(Camera* camera)
	{
		std::unordered_set<Entity*> entities;
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
					if (renderer->GetMesh() && renderer->GetCastsShadows())
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
					if (animator) {
						bones = animator->GetRendererData(renderer->GetUUID())->FinalBoneMatrices;
					}
				}

				Renderer::FlushShadowItem(renderer->GetMesh()->GetVAO(), entity->GetTransform(), bones);
			}
		}
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

		std::vector<Jobs::UIJob> jobs;
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

		std::sort(jobs.begin(), jobs.end(), Jobs::UIJobLess);

		for (const Jobs::UIJob& job : jobs)
			DrawOverlayUIJob(job);

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

		std::vector<Jobs::UIJob> jobs;
		jobs.reserve(256);
		uint64_t traversal = 0;

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
				if (canvasRt)
				{
					const vec2 sceneTargetPixels((float)size.x, (float)size.y);
					const vec2 canvasUnits = scaler->ComputeOverlayCanvasSize(sceneTargetPixels);
					canvasRt->SetWidth(canvasUnits.x);
					canvasRt->SetHeight(canvasUnits.y);
				}
			}

			CollectWorldUIJobsRecursive(entity, canvas->GetSortingLayer(), canvas->GetOrderInLayer(), jobs, traversal);
		}

		std::sort(jobs.begin(), jobs.end(), Jobs::UIJobLess);

		for (const Jobs::UIJob& job : jobs)
			DrawWorldUIJob(job);

		Renderer::DisableBlend();
	}


	void GameModule::ProcessOverlayButtonsInput()
	{
		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		if (m_currentScene)
			m_uiNavigation.UpdateOverlay(*m_currentScene, inputEvent);

		const vec2 mouseLocalPx = inputEvent.GetMousePosition();
		ivec2 size = Application::GetInstance().GetWindow().GetSize();
		const vec2 targetPixels(static_cast<float>(size.x), static_cast<float>(size.y));

		std::vector<Jobs::ButtonJob> buttonJobs;
		buttonJobs.reserve(64);
		uint64_t traversal = 0;

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
			CollectOverlayButtonJobsRecursive(entity, mouseCanvas, canvas->GetSortingLayer(), canvas->GetOrderInLayer(), buttonJobs, traversal);
		}

		std::sort(buttonJobs.begin(), buttonJobs.end(), Jobs::ButtonJobLess);

		const bool down = (inputEvent.GetMouseButtonStatus(0) == KeyState::DOWN) || (inputEvent.GetMouseButtonStatus(0) == KeyState::REPEAT);
		const bool justDown = (inputEvent.GetMouseButtonStatus(0) == KeyState::DOWN);
		const bool up = (inputEvent.GetMouseButtonStatus(0) == KeyState::UP);

		static bool s_pressedInside = false;
		static UUID s_pressedButtonUUID = UUID::Invalid;

		Jobs::ButtonJob* topHovered = nullptr;
		for (auto it = buttonJobs.rbegin(); it != buttonJobs.rend(); ++it)
		{
			if (IsButtonJobHovered(*it))
			{
				topHovered = &(*it);
				break;
			}
		}

		if (justDown && topHovered)
		{
			s_pressedInside = true;
			s_pressedButtonUUID = topHovered->ButtonComp ? topHovered->ButtonComp->GetUUID() : UUID::Invalid;
		}

		for (Jobs::ButtonJob& job : buttonJobs)
		{
			if (!job.ButtonComp)
				continue;

			const bool focused = job.ButtonComp->IsFocused();
			const bool hovered = (topHovered && topHovered->ButtonComp == job.ButtonComp) ? Jobs::IsButtonJobHovered(job) : false;
			job.ButtonComp->SetHovered(hovered || focused);

			const bool isPressedTarget = (UUID::IsValid(s_pressedButtonUUID.Get())) && (job.ButtonComp->GetUUID() == s_pressedButtonUUID);
			job.ButtonComp->SetPressed(down && isPressedTarget && (hovered || focused) && s_pressedInside);
		}

		if (up && s_pressedInside)
		{
			for (Jobs::ButtonJob& job : buttonJobs)
			{
				if (!job.ButtonComp)
					continue;

				if (!(job.ButtonComp->GetUUID() == s_pressedButtonUUID))
					continue;

				const bool hovered = (topHovered && topHovered->ButtonComp == job.ButtonComp) ? Jobs::IsButtonJobHovered(job) : false;
				if (hovered)
					job.ButtonComp->TriggerClick();
				break;
			}

			s_pressedInside = false;
			s_pressedButtonUUID = UUID::Invalid;
		}
	}

}