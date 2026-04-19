#pragma once

#include "Loopie/Core/Module.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"

#include "Loopie/UI/UINavigationSystem.h"

#include "Loopie/Core/Application.h"
#include <memory>

namespace Loopie {

	class Camera;
	class Material;
	class Shader;
	class Canvas;

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
		bool UpdateComponents(DebugGameMode mode);
		void RenderWorld(Camera* camera);
		void RenderParticles(Camera* cam);
		void RenderShadows(Camera* camera);

		void RenderUIRecursive(const std::shared_ptr<Entity>& entity, vec2& scale);
		void RenderUI();
		void RenderSceneUIRecursive(const std::shared_ptr<Entity>& entity);
		void RenderSceneUI(Camera* camera);

		void ProcessOverlayButtonsInput();
	private:

		Scene* m_currentScene = nullptr;
		UINavigationSystem m_uiNavigation;

		DebugGameMode mode;
	};
}