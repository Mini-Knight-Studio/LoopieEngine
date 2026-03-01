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

		void OnNotify(const EngineNotification& type) override;

		void OnUpdate() override;


		static Canvas* FindCanvasInParents(const std::shared_ptr<Loopie::Entity>& entity);
	private:
		bool UpdateComponents(DebugGameMode mode);
		void RenderWorld(Camera* camera);

		void RenderUIRecursive(const std::shared_ptr<Entity>& entity, vec2& scale);
		void RenderUI();
		void RenderSceneUIRecursive(const std::shared_ptr<Entity>& entity);
		void RenderSceneUI(Camera* camera);
	private:

		Scene* m_currentScene = nullptr;

		DebugGameMode mode;
	};
}