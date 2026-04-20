#pragma once

#include "Loopie/Core/Module.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"

#include "Editor/Interfaces/Workspace/InspectorInterface.h"
#include "Editor/Interfaces/Workspace/ConsoleInterface.h"
#include "Editor/Interfaces/Workspace/HierarchyInterface.h"
#include "Editor/Interfaces/Workspace/SceneInterface.h"
#include "Editor/Interfaces/Workspace/GameInterface.h"
#include "Editor/Interfaces/Workspace/EditorMenuInterface.h"
#include "Editor/Interfaces/Workspace/AssetsExplorerInterface.h"
#include "Editor/Interfaces/Workspace/TopBarInterface.h"
#include "Editor/Interfaces/Workspace/TextEditorInterface.h"

#include "Loopie/UI/UINavigationSystem.h"


namespace Loopie {

	class Camera;
	class Material;
	class Shader;

	class EditorModule : public Module, public IObserver<EngineNotification> {
	public:
		EditorModule() = default;
		~EditorModule() = default;

		void OnLoad()override;
		void OnUnload()override;

		void OnNotify(const EngineNotification& type) override;

		void OnUpdate() override;

		void OnInterfaceRender()override;

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

		void SeparateEntities(const std::unordered_set<Entity*>& entities, std::unordered_set<Entity*>& staticEntities, 
							  std::unordered_set<Entity*>& dynamicEntities);

		void RenderUIRecursive(const std::shared_ptr<Entity>& entity, vec2& scale);
		void RenderUI();
		void RenderSceneUIRecursive(const std::shared_ptr<Entity>& entity);
		void RenderSceneUI(Camera* camera);

		void ProcessOverlayButtonsInput();
		static void ProcessOverlayButtonsRecursive(const std::shared_ptr<Loopie::Entity>& entity, const vec2& mouseCanvas, bool mouseOverGame, const Loopie::InputEventManager& input, bool& pressedInsideAny, bool& releasedInsideAny);

	private:
		InspectorInterface m_inspector;
		ConsoleInterface m_console;
		HierarchyInterface m_hierarchy;
		SceneInterface m_scene;
		GameInterface m_game;
		EditorMenuInterface m_mainMenu;
		AssetsExplorerInterface m_assetsExplorer;
		TopBarInterface m_topBar;
		TextEditorInterface m_textEditor;

		Scene* m_currentScene = nullptr;
		UINavigationSystem m_uiNavigation;
		std::shared_ptr<Material> m_selectedObjectMaterial;
		Shader* m_selectedObjectShader;
	};
}