#pragma once

#include "Loopie/Core/UUID.h"

#include <memory>

namespace Loopie
{
	class InputEventManager;
	class Scene;
	class UIElement;
	class Entity;

	class UINavigationSystem
	{
	public:
		UINavigationSystem() = default;

		void UpdateOverlay(Scene& scene, const InputEventManager& input);

		bool HasFocus() const { return !(m_focusedEntity == UUID::Invalid); }
		const UUID& GetFocusedEntity() const { return m_focusedEntity; }

		void ClearFocus(Scene& scene);
		bool SetFocus(Scene& scene, const UUID& entity);

	private:
		UIElement* GetFocusedElement(Scene& scene) const;
		bool FocusFirstOverlayElement(Scene& scene);
		static UIElement* FindFirstFocusableOverlayElement(Scene& scene);
		static UIElement* FindFirstFocusableInTree(const std::shared_ptr<Entity>& root);
		static UIElement* FindFocusableOnEntity(const std::shared_ptr<Entity>& entity);

		bool MoveFocus(Scene& scene, int dir);
		void Submit(Scene& scene);
		void Back(Scene& scene);

	private:
		UUID m_focusedEntity = UUID::Invalid;
	};
}
