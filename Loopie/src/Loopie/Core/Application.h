#pragma once

#include "Loopie/Core/Module.h"
#include "Loopie/Core/Window.h"
#include "Loopie/Core/InputEventManager.h"
#include "Loopie/Events/Event.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Project/Project.h"
#include "Loopie/ImGui/ImGuiManager.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Core/Clipboard.h"

#include <vector>

namespace Loopie {
	class Application : public IObserver<EngineNotification>{
	public:
		Application();
		~Application();

		void AddModule(Module* module);
		void RemoveModule(Module* module);

		void Run();
		void Close();

		static Application& GetInstance();
		Window& GetWindow();
		InputEventManager& GetInputEvent();
		Scene& GetScene();

		void SetInterfaceState(bool enabled) { m_renderInterface = enabled; }
		bool IsInterfaceVisible() const { return m_renderInterface; }

		void CreateScene();
	private:
		void ProcessEvents(InputEventManager& eventController);

		void OnNotify(const EngineNotification& id) override;

	public:
		Project m_activeProject;
		Event<EngineNotification> m_notifier;
		Clipboard<std::string> m_clipboard;
	private:
		static Application* s_Instance;
		Scene* m_scene = nullptr;

		std::vector<Module*> m_modules;
		Window* m_window = nullptr;

		ImGuiManager m_imguiManager;

		InputEventManager m_inputEvent;

		bool m_running = true;
		bool m_renderInterface = true;

		
	};

	//// Define Main
	/*int main() {
		Application* app = new "ApplicationType()";
		app->Run();
		delete app;
	}*/
}