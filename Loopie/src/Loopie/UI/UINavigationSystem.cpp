#include "Loopie/UI/UINavigationSystem.h"

#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/UIElement.h"
#include "Loopie/Core/InputEventManager.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Scene/Entity.h"

#include <SDL3/SDL_scancode.h>

#include <vector>
#include <limits>

namespace Loopie
{
	struct NavCandidate
	{
		UUID entity;
		vec2 center;
	};

	static bool IsDown(KeyState s)
	{
		return s == KeyState::DOWN;
	}

	static bool TryGetRectCenter(const std::shared_ptr<Entity>& entity, vec2& outCenter)
	{
		if (!entity)
			return false;
		RectTransform* rt = entity->GetComponent<RectTransform>();
		if (!rt || !rt->GetIsActive())
			return false;

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

		outCenter = vec2((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);
		return true;
	}

	static void CollectFocusableOverlayCandidates(Scene& scene, std::vector<NavCandidate>& out)
	{
		out.clear();
		out.reserve(scene.GetAllEntities().size());

		for (const auto& [uuid, entity] : scene.GetAllEntities())
		{
			if (!entity || !entity->GetIsActiveInHierarchy())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			if (!canvas || !canvas->GetIsActive())
				continue;
			if (canvas->GetRenderMode() != CanvasRenderMode::ScreenSpaceOverlay)
				continue;

			std::vector<std::shared_ptr<Entity>> stack;
			stack.push_back(entity);
			while (!stack.empty())
			{
				std::shared_ptr<Entity> current = stack.back();
				stack.pop_back();

				if (current && current->GetIsActiveInHierarchy())
				{
					UIElement* focusable = nullptr;
					for (Component* component : current->GetComponents())
					{
						UIElement* ui = component ? component->AsUIElement() : nullptr;
						if (ui && ui->CanFocus())
						{
							focusable = ui;
							break;
						}
					}

					if (focusable)
					{
						vec2 center;
						if (TryGetRectCenter(current, center))						
							out.push_back({ current->GetUUID(), center });
					}
				}

				for (const auto& child : current->GetChildren())
					stack.push_back(child);
			}
		}
	}

	static bool FindBestNeighbor(Scene& scene, const UUID& currentEntity, UINavigationDirection dir, UUID& outNeighbor)
	{
		outNeighbor = UUID::Invalid;

		auto current = scene.GetEntity(currentEntity);
		if (!current)
			return false;

		vec2 currentCenter;
		if (!TryGetRectCenter(current, currentCenter))
			return false;

		std::vector<NavCandidate> candidates;
		CollectFocusableOverlayCandidates(scene, candidates);

		float bestScore = std::numeric_limits<float>::infinity();
		UUID bestEntity = UUID::Invalid;

		for (const NavCandidate& c : candidates)
		{
			if (c.entity == currentEntity)
				continue;

			const vec2 d = c.center - currentCenter;

			float parallel = 0.0f;
			float perpendicular = 0.0f;
			switch (dir)
			{
			case UINavigationDirection::Up:
				if (d.y <= 0.001f) continue;
				parallel = d.y;
				perpendicular = glm::abs(d.x);
				break;
			case UINavigationDirection::Down:
				if (d.y >= -0.001f) continue;
				parallel = -d.y;
				perpendicular = glm::abs(d.x);
				break;
			case UINavigationDirection::Left:
				if (d.x >= -0.001f) continue;
				parallel = -d.x;
				perpendicular = glm::abs(d.y);
				break;
			case UINavigationDirection::Right:
				if (d.x <= 0.001f) continue;
				parallel = d.x;
				perpendicular = glm::abs(d.y);
				break;
			}

			const float score = (perpendicular * perpendicular) * 4.0f + (parallel * parallel);
			if (score < bestScore)
			{
				bestScore = score;
				bestEntity = c.entity;
			}
		}

		if (bestEntity == UUID::Invalid)
			return false;

		outNeighbor = bestEntity;
		return true;
	}

	static bool HasNavInputThisFrame(const InputEventManager& input)
	{
		return IsDown(input.GetKeyStatus(SDL_SCANCODE_UP)) ||
			IsDown(input.GetKeyStatus(SDL_SCANCODE_DOWN)) ||
			IsDown(input.GetKeyStatus(SDL_SCANCODE_LEFT)) ||
			IsDown(input.GetKeyStatus(SDL_SCANCODE_RIGHT)) ||
			IsDown(input.GetKeyStatus(SDL_SCANCODE_RETURN)) ||
			IsDown(input.GetKeyStatus(SDL_SCANCODE_SPACE)) ||
			IsDown(input.GetKeyStatus(SDL_SCANCODE_ESCAPE)) ||
			IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_UP)) ||
			IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_DOWN)) ||
			IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_LEFT)) ||
			IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) ||
			IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_SOUTH)) ||
			IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_EAST));
	}

	void UINavigationSystem::UpdateOverlay(Scene& scene, const InputEventManager& input)
	{
		if (!HasNavInputThisFrame(input))
			return;

		if (UIElement* focused = GetFocusedElement(scene); !focused || !focused->CanFocus())
		{
			ClearFocus(scene);
			FocusFirstOverlayElement(scene);
		}

		if (!HasFocus())
			return;

		const bool up = IsDown(input.GetKeyStatus(SDL_SCANCODE_UP)) || IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_UP));
		const bool down = IsDown(input.GetKeyStatus(SDL_SCANCODE_DOWN)) || IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_DOWN));
		const bool left = IsDown(input.GetKeyStatus(SDL_SCANCODE_LEFT)) || IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_LEFT));
		const bool right = IsDown(input.GetKeyStatus(SDL_SCANCODE_RIGHT)) || IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_DPAD_RIGHT));

		if (up)
			MoveFocus(scene, static_cast<int>(UINavigationDirection::Up));
		else if (down)
			MoveFocus(scene, static_cast<int>(UINavigationDirection::Down));
		else if (left)
			MoveFocus(scene, static_cast<int>(UINavigationDirection::Left));
		else if (right)
			MoveFocus(scene, static_cast<int>(UINavigationDirection::Right));

		const bool submit = IsDown(input.GetKeyStatus(SDL_SCANCODE_RETURN)) || IsDown(input.GetKeyStatus(SDL_SCANCODE_SPACE)) || IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_SOUTH));
		const bool back = IsDown(input.GetKeyStatus(SDL_SCANCODE_ESCAPE)) || IsDown(input.GetGamepadButtonStatus(SDL_GAMEPAD_BUTTON_EAST));

		if (submit)
			Submit(scene);
		if (back)
			Back(scene);
	}

	void UINavigationSystem::ClearFocus(Scene& scene)
	{
		if (UIElement* focused = GetFocusedElement(scene))
			focused->Blur();
		m_focusedEntity = UUID::Invalid;
	}

	bool UINavigationSystem::SetFocus(Scene& scene, const UUID& entity)
	{
		auto targetEntity = scene.GetEntity(entity);
		UIElement* target = FindFocusableOnEntity(targetEntity);
		if (!target)
			return false;

		if (UIElement* focused = GetFocusedElement(scene))
			focused->Blur();

		m_focusedEntity = entity;
		target->Focus();
		return true;
	}

	UIElement* UINavigationSystem::GetFocusedElement(Scene& scene) const
	{
		if (m_focusedEntity == UUID::Invalid)
			return nullptr;

		auto entity = scene.GetEntity(m_focusedEntity);
		if (!entity)
			return nullptr;

		for (Component* component : entity->GetComponents())
		{
			UIElement* ui = component ? component->AsUIElement() : nullptr;
			if (!ui)
				continue;
			if (ui->IsFocused())
				return ui;
		}

		return FindFocusableOnEntity(entity);
	}

	bool UINavigationSystem::FocusFirstOverlayElement(Scene& scene)
	{
		UIElement* first = FindFirstFocusableOverlayElement(scene);
		if (!first)
			return false;

		auto owner = first->GetOwner();
		if (!owner)
			return false;

		m_focusedEntity = owner->GetUUID();
		first->Focus();
		return true;
	}

	UIElement* UINavigationSystem::FindFirstFocusableOverlayElement(Scene& scene)
	{
		for (const auto& [uuid, entity] : scene.GetAllEntities())
		{
			if (!entity || !entity->GetIsActiveInHierarchy())
				continue;

			Canvas* canvas = entity->GetComponent<Canvas>();
			if (!canvas || !canvas->GetIsActive())
				continue;
			if (canvas->GetRenderMode() != CanvasRenderMode::ScreenSpaceOverlay)
				continue;

			if (UIElement* ui = FindFirstFocusableInTree(entity))
				return ui;
		}

		return nullptr;
	}

	UIElement* UINavigationSystem::FindFirstFocusableInTree(const std::shared_ptr<Entity>& root)
	{
		if (!root)
			return nullptr;

		if (UIElement* ui = FindFocusableOnEntity(root))
			return ui;

		for (const auto& child : root->GetChildren())
		{
			if (UIElement* ui = FindFirstFocusableInTree(child))
				return ui;
		}

		return nullptr;
	}

	UIElement* UINavigationSystem::FindFocusableOnEntity(const std::shared_ptr<Entity>& entity)
	{
		if (!entity || !entity->GetIsActiveInHierarchy())
			return nullptr;

		for (Component* component : entity->GetComponents())
		{
			UIElement* ui = component ? component->AsUIElement() : nullptr;
			if (!ui)
				continue;
			if (ui->CanFocus())
				return ui;
		}

		return nullptr;
	}

	bool UINavigationSystem::MoveFocus(Scene& scene, int dir)
	{
		UIElement* current = GetFocusedElement(scene);
		if (!current)
			return FocusFirstOverlayElement(scene);

		UUID neighbor = current->GetNeighborEntity(static_cast<UINavigationDirection>(dir));
		if (!(neighbor == UUID::Invalid))
			return SetFocus(scene, neighbor);

		UUID autoNeighbor;
		if (!FindBestNeighbor(scene, m_focusedEntity, static_cast<UINavigationDirection>(dir), autoNeighbor))
			return false;

		return SetFocus(scene, autoNeighbor);
	}

	void UINavigationSystem::Submit(Scene& scene)
	{
		if (UIElement* current = GetFocusedElement(scene))
			current->Submit();
	}

	void UINavigationSystem::Back(Scene& scene)
	{
		if (UIElement* current = GetFocusedElement(scene))
			current->Back();
	}
}
