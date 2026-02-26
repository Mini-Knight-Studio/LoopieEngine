#include "GameLogic.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Components/ScriptClass.h"

#include <imgui.h>


namespace Loopie {

	GameLogic::GameLogic() {
		m_visible = true;
	}

	GameLogic::~GameLogic() {

		for (const auto& [uuid, entity] : Application::GetInstance().GetScene().GetAllEntities())
		{
			if (!entity->HasComponent<ScriptClass>())
				continue;
			for (auto& component : entity->GetComponents<ScriptClass>())
			{
				component->DestroyInstance();
			}
		}

		ScriptingManager::RuntimeStop();
	}

	void GameLogic::Init() {
		ScriptingManager::RuntimeStart();
		for (const auto& [uuid, entity] : Application::GetInstance().GetScene().GetAllEntities())
		{
			if (!entity->HasComponent<ScriptClass>())
				continue;
			for (auto& component : entity->GetComponents<ScriptClass>())
			{
				component->InvokeOnCreate();
			}
		}
	}

	void GameLogic::Update(const InputEventManager& inputEvent)
	{
		for (auto& [uuid, entity] : Application::GetInstance().GetScene().GetAllEntities())
		{
			if (entity == nullptr)
				continue;
			if (!entity->HasComponent< ScriptClass>())
				continue;

			for (auto& component : entity->GetComponents<ScriptClass>())
			{
				component->InvokeOnUpdate();
			}
		}
	}

	Camera* GameLogic::GetCamera()
	{
		return Camera::GetMainCamera();
	}
}