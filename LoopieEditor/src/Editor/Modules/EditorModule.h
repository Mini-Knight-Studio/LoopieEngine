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

#include "Loopie/ParticleSystemEn/ParticleSystem.h"  
#include "Loopie/Components/ParticleComponent.h"

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
		void RenderWorld(Camera* camera);
		void RenderParticles(Camera* cam);
		/// Test
		void CreateBakerHouse();
		void CreateCity();
		//void MousePick(Camera* camera);
		///
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
		std::shared_ptr<Material> m_selectedObjectMaterial;
		Shader* m_selectedObjectShader;
	};
}