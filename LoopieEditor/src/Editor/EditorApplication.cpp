#include "Editor/Modules/ProjectSetupModule.h"
#include "Loopie/Scripting/ScriptingManager.h"

#include "Loopie/Core/Application.h"

namespace Loopie {
	class EditorApplication : public Application {
	public:
		EditorApplication() : Application() {
			//// Open first the module -> Project CreationSetUp
			//// Once the Porject is selected, remove the module and add the EditorModule
			AddModule(new ProjectSetupModule());
			GetWindow().SetLogo("assets/logo/logo.bmp");
		}

		void Quit() override
		{
			StopScripting(true);
		}
	};
}

int main() {
	Loopie::Application* app = new Loopie::EditorApplication();
	app->Run();
	delete app;
}