#pragma once

#include "Editor/Interfaces/Interface.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Resources/Types/Texture.h"
namespace Loopie
{
	enum DebugGameMode
	{
		START,
		UPDATING,
		PAUSED,
		END,
		NEXTFRAME,
		GIZMO,
		DEACTIVATED,
	};
	class TopBarInterface : public Interface
	{
	public:
		TopBarInterface();
		~TopBarInterface() = default;
		void Init() override {}
		void Update(const InputEventManager& inputEvent) override;
		void Render() override;

		float GetToolbarHeight() const { return m_toolbarHeight; }

		DebugGameMode GetCurrentMode() const { return m_actualMode; }
		void SetMode(DebugGameMode mode) { m_actualMode = mode; }
	private:
		std::shared_ptr<Texture> m_playIcon;
		std::shared_ptr<Texture> m_pauseIcon;
		std::shared_ptr<Texture> m_stopIcon;
		std::shared_ptr<Texture> m_nextFrameIcon;
		DebugGameMode m_actualMode = DEACTIVATED;

		float m_toolbarHeight = 32.0f;
	};
}