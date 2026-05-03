#include "UIManager.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/InputEventManager.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Scripting/ScriptingManager.h"

#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/CanvasScaler.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/UIElement.h"

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_gamepad.h>

#include <algorithm>

namespace Loopie
{
	struct UIManager::PickCandidate
	{
		UUID EntityUUID = UUID::Invalid;
		int CanvasSortingLayer = 0;
		int CanvasOrderInLayer = 0;
		int ElementSortingLayer = 0;
		int ElementOrderInLayer = 0;
		uint64_t TraversalIndex = 0;
	};

	UIElement* UIManager::FindUIElementComponent(const std::shared_ptr<Entity>& entity)
	{
		if (!entity)
			return nullptr;

		const std::vector<Component*>& components = entity->GetComponents();
		for (Component* component : components)
		{
			if (!component)
				continue;

			UIElement* ui = component->AsUIElement();
			if (!ui)
				continue;
			if (ui->CanFocus())
				return ui;
		}

		return nullptr;
	}

	bool UIManager::CandidateLess(const PickCandidate& a, const PickCandidate& b)
	{
		if (a.CanvasSortingLayer != b.CanvasSortingLayer) return a.CanvasSortingLayer < b.CanvasSortingLayer;
		if (a.CanvasOrderInLayer != b.CanvasOrderInLayer) return a.CanvasOrderInLayer < b.CanvasOrderInLayer;
		if (a.ElementSortingLayer != b.ElementSortingLayer) return a.ElementSortingLayer < b.ElementSortingLayer;
		if (a.ElementOrderInLayer != b.ElementOrderInLayer) return a.ElementOrderInLayer < b.ElementOrderInLayer;
		return a.TraversalIndex < b.TraversalIndex;
	}

	void UIManager::CollectOverlayPickCandidatesRecursive(const std::shared_ptr<Entity>& entity, const vec2& mouseCanvas,
		int canvasSortingLayer, int canvasOrderInLayer,
		std::vector<PickCandidate>& outCandidates, uint64_t& inOutTraversal)
	{
		if (!entity || !entity->GetIsActive())
			return;

		RectTransform* rt = entity->GetComponent<RectTransform>();
		UIElement* ui = FindUIElementComponent(entity);
		if (rt && ui && ui->GetIsActive() && ui->CanFocus())
		{
			const vec3 p = rt->GetWorldPosition();
			const vec3 ws3 = rt->GetWorldScale();
			const vec2 ws(ws3.x, ws3.y);
			const vec3 bmin = rt->GetLocalBoundsMin();
			const vec3 bmax = rt->GetLocalBoundsMax();

			const float x0 = p.x + bmin.x * ws.x;
			const float y0 = p.y + bmin.y * ws.y;
			const float x1 = p.x + bmax.x * ws.x;
			const float y1 = p.y + bmax.y * ws.y;

			const float minX = glm::min(x0, x1);
			const float maxX = glm::max(x0, x1);
			const float minY = glm::min(y0, y1);
			const float maxY = glm::max(y0, y1);

			const bool hovered = (mouseCanvas.x >= minX && mouseCanvas.x <= maxX &&
				mouseCanvas.y >= minY && mouseCanvas.y <= maxY);

			if (hovered)
			{
				outCandidates.push_back(PickCandidate{
					entity->GetUUID(),
					canvasSortingLayer,
					canvasOrderInLayer,
					ui->GetSortingLayer(),
					ui->GetOrderInLayer(),
					inOutTraversal++
				});
			}
		}

		for (const auto& child : entity->GetChildren())
			CollectOverlayPickCandidatesRecursive(child, mouseCanvas, canvasSortingLayer, canvasOrderInLayer, outCandidates, inOutTraversal);
	}

	bool UIManager::TryApplyDeserializedSelection(Scene& scene, const UUID& desired)
	{
		if (desired == UUID::Invalid)
			return true;

		std::shared_ptr<Entity> entity = scene.GetEntity(desired);
		UIElement* ui = FindUIElementComponent(entity);
		if (!ui || !ui->GetIsActive() || !ui->CanFocus())
			return false;

		ui->Focus();
		return true;
	}

	void UIManager::SetExternalMouseSelectionContext(const vec2& mouseLocalPx, const ivec2& targetPixels, bool enabled)
	{
		m_hasExternalMouseSelectionContext = true;
		m_externalMouseSelectionEnabled = enabled;
		m_externalMouseLocalPx = mouseLocalPx;
		m_externalTargetPixels = targetPixels;
	}

	void UIManager::ClearExternalMouseSelectionContext()
	{
		m_hasExternalMouseSelectionContext = false;
		m_externalMouseSelectionEnabled = true;
		m_externalMouseLocalPx = { 0.0f, 0.0f };
		m_externalTargetPixels = { 0, 0 };
	}

	UIManager::UIManager()
	{
		// Default Keyboard
		m_moveUpBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_W });
		m_moveUpBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_UP });

		m_moveDownBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_S });
		m_moveDownBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_DOWN });

		m_moveLeftBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_A });
		m_moveLeftBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_LEFT });

		m_moveRightBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_D });
		m_moveRightBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_RIGHT });

		m_selectBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_RETURN });
		m_selectBindings.push_back({ BindingType::KeyboardScancode, SDL_SCANCODE_SPACE });

		// Default Gamepad
		m_moveUpBindings.push_back({ BindingType::GamepadButton, SDL_GAMEPAD_BUTTON_DPAD_UP });
		m_moveUpBindings.push_back({ BindingType::GamepadAxis, SDL_GAMEPAD_AXIS_LEFTY, -1.0f });

		m_moveDownBindings.push_back({ BindingType::GamepadButton, SDL_GAMEPAD_BUTTON_DPAD_DOWN });
		m_moveDownBindings.push_back({ BindingType::GamepadAxis, SDL_GAMEPAD_AXIS_LEFTY,  1.0f });

		m_moveLeftBindings.push_back({ BindingType::GamepadAxis, SDL_GAMEPAD_AXIS_LEFTX, -1.0f });
		m_moveLeftBindings.push_back({ BindingType::GamepadButton, SDL_GAMEPAD_BUTTON_DPAD_LEFT });

		m_moveRightBindings.push_back({ BindingType::GamepadButton, SDL_GAMEPAD_BUTTON_DPAD_RIGHT });
		m_moveRightBindings.push_back({ BindingType::GamepadAxis, SDL_GAMEPAD_AXIS_LEFTX, 1.0f });

		m_selectBindings.push_back({ BindingType::GamepadButton, SDL_GAMEPAD_BUTTON_SOUTH });
	}

	void UIManager::OnUpdate()
	{
		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		if (m_applySelectionOnSceneDeserialized)
		{
			Scene& scene = app.GetScene();
			if (TryApplyDeserializedSelection(scene, m_selectedEntity))
				m_applySelectionOnSceneDeserialized = false;
		}

		if (!ScriptingManager::IsRunning())
			return;
		if (inputEvent.IsReadingInputText())
			return;

		HandleMouseSelection();

		if (m_selectedEntity == UUID::Invalid)
			return;

		if (AnyPressed(m_moveUpBindings))
			TryMove(UINavigationDirection::Up);
		else if (AnyPressed(m_moveDownBindings))
			TryMove(UINavigationDirection::Down);
		else if (AnyPressed(m_moveLeftBindings))
			TryMove(UINavigationDirection::Left);
		else if (AnyPressed(m_moveRightBindings))
			TryMove(UINavigationDirection::Right);

		if (AnyPressed(m_selectBindings))
			TrySubmit();
	}

	void UIManager::OnSceneDeserialized()
	{
		if (!m_applySelectionOnSceneDeserialized)
			return;

		Scene& scene = Application::GetInstance().GetScene();
		if (TryApplyDeserializedSelection(scene, m_selectedEntity))
			m_applySelectionOnSceneDeserialized = false;
	}

	JsonNode UIManager::Serialize(JsonNode& parent) const
	{
		JsonNode node = parent.CreateObjectField("ui_manager");

		node.CreateField<std::string>("selected_entity", (m_selectedEntity == UUID::Invalid) ? std::string() : m_selectedEntity.Get());

		auto writeBindings = [&](const char* field, const std::vector<InputBinding>& bindings)
		{
			JsonNode arr = node.CreateArrayField(field);
			for (const InputBinding& binding : bindings)
			{
				json j = json::object();
				j["type"] = (int)binding.Type;
				j["code"] = binding.Code;
				j["axis_dir"] = binding.AxisDirection;
				arr.AddArrayElement(j);
			}
		};

		writeBindings("move_up", m_moveUpBindings);
		writeBindings("move_down", m_moveDownBindings);
		writeBindings("move_left", m_moveLeftBindings);
		writeBindings("move_right", m_moveRightBindings);
		writeBindings("select", m_selectBindings);

		return node;
	}

	void UIManager::Deserialize(const JsonNode& data)
	{
		const std::string selected = data.GetValue<std::string>("selected_entity", "").Result;
		m_selectedEntity = selected.empty() ? UUID::Invalid : UUID(selected);
		m_applySelectionOnSceneDeserialized = true;

		auto readBindings = [&](const char* field, std::vector<InputBinding>& out)
		{
			out.clear();
			JsonNode arr = data.Child(field);
			if (!arr.IsValid() || !arr.IsArray())
				return;

			const unsigned int n = arr.Size();
			out.reserve(n);
			for (unsigned int i = 0; i < n; ++i)
			{
				JsonResult<json> el = arr.GetArrayElement<json>(i);
				if (!el.Found || !el.Result.is_object())
					continue;

				const int type = el.Result.value("type", 0);
				const int code = el.Result.value("code", 0);
				const float direction = el.Result.value("axis_dir", 1.0f);

				InputBinding binding;
				switch (type)
				{
					case 0:
						binding.Type = BindingType::KeyboardScancode;
						binding.Code = code;
						break;
					case 1:
						binding.Type = BindingType::GamepadButton;
						binding.Code = code;
						break;
					case 2:		
						binding.Type = BindingType::GamepadAxis;
						binding.Code = code;
						binding.AxisDirection = direction;
						break;
					default:
						binding.Type = BindingType::KeyboardScancode;
						binding.Code = 0;
						break;
				}

				out.push_back(binding);
			}
		};

		readBindings("move_up", m_moveUpBindings);
		readBindings("move_down", m_moveDownBindings);
		readBindings("move_left", m_moveLeftBindings);
		readBindings("move_right", m_moveRightBindings);
		readBindings("select", m_selectBindings);
	}

	void UIManager::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const UIManager& otherMgr = static_cast<const UIManager&>(other);
		m_selectedEntity = otherMgr.m_selectedEntity;
		m_moveUpBindings = otherMgr.m_moveUpBindings;
		m_moveDownBindings = otherMgr.m_moveDownBindings;
		m_moveLeftBindings = otherMgr.m_moveLeftBindings;
		m_moveRightBindings = otherMgr.m_moveRightBindings;
		m_selectBindings = otherMgr.m_selectBindings;

		m_applySelectionOnSceneDeserialized = true;
	}

	void UIManager::SetSelectedEntity(const UUID& entityUUID)
	{
		if (entityUUID == m_selectedEntity)
			return;

		Scene& scene = Application::GetInstance().GetScene();

		if (!(m_selectedEntity == UUID::Invalid))
		{
			std::shared_ptr<Entity> oldEntity = scene.GetEntity(m_selectedEntity);
			if (UIElement* oldUI = FindUIElementComponent(oldEntity))
				oldUI->Blur();
		}

		m_selectedEntity = UUID::Invalid;

		if (entityUUID == UUID::Invalid)
			return;

		std::shared_ptr<Entity> newEntity = scene.GetEntity(entityUUID);
		UIElement* newUI = FindUIElementComponent(newEntity);
		if (!newUI || !newUI->GetIsActive() || !newUI->CanFocus())
			return;

		m_selectedEntity = entityUUID;
		newUI->Focus();
	}

	void UIManager::ClearSelection()
	{
		SetSelectedEntity(UUID::Invalid);
	}

	bool UIManager::AnyPressed(const std::vector<InputBinding>& bindings) const
	{
		if (bindings.empty())
			return false;

		InputEventManager& input = Application::GetInstance().GetInputEvent();

		const float threshold = 0.5f;

		for (const InputBinding& b : bindings)
		{
			switch (b.Type)
			{
				case BindingType::KeyboardScancode:
					if (input.GetKeyStatus((SDL_Scancode)b.Code) == KeyState::DOWN)
						return true;
					break;

				case BindingType::GamepadButton:
					if (input.GetGamepadButtonStatus((SDL_GamepadButton)b.Code) == KeyState::DOWN)
						return true;
					break;

				case BindingType::GamepadAxis:
				{
					float value = input.GetAxisValueRaw((SDL_GamepadAxis)b.Code);
					value *= b.AxisDirection;

					bool active = value > threshold;

					AxisKey key{ b.Code, b.AxisDirection };

					bool wasActive = m_axisWasActive[key];
					m_axisWasActive[key] = active;

					if (active && !wasActive)
						return true;

					break;
				}
			}
		}

		return false;
	}

	void UIManager::TryMove(UINavigationDirection dir)
	{
		Scene& scene = Application::GetInstance().GetScene();

		std::shared_ptr<Entity> currentEntity = scene.GetEntity(m_selectedEntity);
		UIElement* currentUI = FindUIElementComponent(currentEntity);
		if (!currentUI)
		{
			ClearSelection();
			return;
		}

		const UUID neighborUUID = currentUI->GetNeighborEntity(dir);
		if (neighborUUID == UUID::Invalid)
			return;

		std::shared_ptr<Entity> neighborEntity = scene.GetEntity(neighborUUID);
		UIElement* neighborUI = FindUIElementComponent(neighborEntity);
		if (!neighborUI || !neighborUI->GetIsActive() || !neighborUI->CanFocus())
			return;

		SetSelectedEntity(neighborUUID);
	}

	void UIManager::TrySubmit()
	{
		Scene& scene = Application::GetInstance().GetScene();

		std::shared_ptr<Entity> currentEntity = scene.GetEntity(m_selectedEntity);
		UIElement* currentUI = FindUIElementComponent(currentEntity);
		if (!currentUI || !currentUI->GetIsActive() || !currentUI->CanFocus())
			return;

		currentUI->Submit();
	}

	void UIManager::HandleMouseSelection()
	{
		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();

		const bool justDown = (inputEvent.GetMouseButtonStatus(0) == KeyState::DOWN);
		if (!justDown)
			return;

		bool allowMouseSelection = true;
		ivec2 targetSize = app.GetWindow().GetSize();
		vec2 mouseLocalPx = inputEvent.GetMousePosition();
		if (m_hasExternalMouseSelectionContext)
		{
			allowMouseSelection = m_externalMouseSelectionEnabled;
			targetSize = m_externalTargetPixels;
			mouseLocalPx = m_externalMouseLocalPx;
		}

		if (!allowMouseSelection)
			return;

		if (targetSize.x <= 0 || targetSize.y <= 0)
		{
			ClearSelection();
			return;
		}

		Scene& scene = app.GetScene();

		const vec2 targetPixels((float)targetSize.x, (float)targetSize.y);

		std::vector<PickCandidate> candidates;
		candidates.reserve(32);

		for (const auto& [uuid, entity] : scene.GetAllEntities())
		{
			if (!entity || !entity->GetIsActive())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			RectTransform* canvasRt = entity->GetComponent<RectTransform>();
			if (!canvas || !canvasRt || !canvas->GetIsActive())
				continue;
			if (canvas->GetRenderMode() != CanvasRenderMode::ScreenSpaceOverlay)
				continue;

			vec2 canvasUnits(canvasRt->GetWidth(), canvasRt->GetHeight());
			if (auto* scaler = entity->GetComponent<CanvasScaler>(); scaler && scaler->GetIsActive())
			{
				canvasUnits = scaler->ComputeOverlayCanvasSize(targetPixels);
			}
			else
			{
				canvasUnits = targetPixels;
			}

			const float cw = canvasUnits.x;
			const float ch = canvasUnits.y;
			if (cw <= 0.0f || ch <= 0.0f)
				continue;

			const float sx = targetPixels.x / cw;
			const float sy = targetPixels.y / ch;

			const vec2 mouseCanvas(mouseLocalPx.x / sx, ch - (mouseLocalPx.y / sy));

			uint64_t traversal = 0;
			CollectOverlayPickCandidatesRecursive(entity, mouseCanvas, canvas->GetSortingLayer(), canvas->GetOrderInLayer(), candidates, traversal);
		}

		if (candidates.empty())
		{
			ClearSelection();
			return;
		}

		auto it = std::max_element(candidates.begin(), candidates.end(), CandidateLess);
		if (it == candidates.end())
		{
			ClearSelection();
			return;
		}

		SetSelectedEntity(it->EntityUUID);
	}
}
