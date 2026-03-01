//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include "ProjectBuild/Modules/GameModule.h"

#include "Loopie/Core/Application.h"

namespace Loopie {
	class EditorApplication : public Application {
	public:
		EditorApplication() : Application() {
			AddModule(new GameModule());
		}
	};
}

int main() {
	Loopie::Application* app = new Loopie::EditorApplication();
	app->Run();
	delete app;
}