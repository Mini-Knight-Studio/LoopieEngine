#pragma once

#include "Loopie/Core/Module.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"

#include "ProjectBuild/Interfaces/GameLogic.h"

namespace Loopie {

	class Camera;
	class Material;
	class Shader;

	class GameModule : public Module, public IObserver<EngineNotification> {
	public:
		GameModule() = default;
		~GameModule() = default;

		void OnLoad()override;
		void OnUnload()override;

		void OnNotify(const EngineNotification& type) override;

		void OnUpdate() override;

		void OnInterfaceRender()override;
	private:
		void RenderWorld(Camera* camera);
	private:

		GameLogic m_game;

		Scene* m_currentScene = nullptr;
	};
}