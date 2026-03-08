#pragma once 

#include "Editor/Interfaces/Interface.h"

#include <string>
#include <vector>

namespace Loopie {
	class EditorMenuInterface : public Interface {
	public:
		EditorMenuInterface();
		~EditorMenuInterface() = default;
		void Init() override {}
		void Update(const InputEventManager& inputEvent) override;
		void Render() override;

	private:
		void RenderAboutMenu();
		void RenderInfoConfigMenu();
		void RenderCollisionMatrixMenu();


		void HotKeys(const InputEventManager& inputEvent);

		void RenderOpenProjectPopUp();
		void RenderCreateProjectPopUp();
		void RenderSaveScenePopUp();
		void RenderLoadScenePopUp();

	private:
		char m_projectName[128] = "";
		char m_sceneName[128] = "";
		std::string m_projectPath = "";
		std::string m_newScenePath = "";


		/// Menus States
		bool m_showAboutMenu = false;
		bool m_showInfoConfigMenu = false;
		bool m_showCollisionMatrixMenu = false;


		/// Plot
		const int LOG_SIZE = 100;
		std::vector<float> m_fpsLog;
		std::vector<float> m_msLog;
	};
}