#pragma once

#include "Loopie/Core/Module.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"

#include "Loopie/Core/Application.h"
#include <memory>

namespace Loopie {

	class Camera;
	class Material;
	class Shader;

	enum DebugGameMode
	{
		START,
		UPDATING,
		PAUSED,
		END,
		NEXTFRAME,
		DEACTIVATED,
	};

	class GameModule : public Module, public IObserver<EngineNotification> {
	public:
		GameModule() = default;
		~GameModule() = default;

		void OnLoad()override;
		void OnUnload()override;

		void OnNotify(const EngineNotification& type) override {};

		void OnUpdate() override;

	private:

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

		static void CollectOverlayUIJobsRecursive(const std::shared_ptr<Entity>& entity, const vec2& overlayScale,
			int canvasSortingLayer, int canvasOrderInLayer,
			std::vector<UIJob>& outJobs, uint64_t& inOutTraversal);
		static void DrawOverlayUIJob(const UIJob& job);

		static void CollectWorldUIJobsRecursive(const std::shared_ptr<Entity>& entity,
			int canvasSortingLayer, int canvasOrderInLayer,
			std::vector<UIJob>& outJobs, uint64_t& inOutTraversal);
		static void DrawWorldUIJob(const UIJob& job);

		bool UpdateComponents(DebugGameMode mode);
		void RenderWorld(Camera* camera);
		void RenderShadows(const Camera* cam);
		void RenderEntityShadows(const std::unordered_set<Entity*>& entities);
		void RenderParticles(Camera* cam);

		void SeparateDynamicEntities(const std::unordered_set<Entity*>& entities, std::unordered_set<Entity*>& dynamicEntities);

		void RenderUI();
		void RenderSceneUIRecursive(const std::shared_ptr<Entity>& entity);
		void RenderSceneUI(Camera* camera);

		void ProcessOverlayButtonsInput();
		void ProcessOverlayButtonsRecursive(const std::shared_ptr<Loopie::Entity>& entity, const vec2& mouseCanvas, bool mouseOverGame, const Loopie::InputEventManager& input, bool& pressedInsideAny, bool& releasedInsideAny);
	private:

		Scene* m_currentScene = nullptr;

		DebugGameMode mode;
	};
}