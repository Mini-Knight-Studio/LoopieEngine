#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/UI/UINavigable.h"

#include "Loopie/Scene/Entity.h"

#include <array>

namespace Loopie
{
	class UIElement : public Component, public UINavigable
	{
	public:
		DEFINE_TYPE(UIElement)

		UIElement() = default;
		~UIElement() override = default;

		UIElement* AsUIElement() override { return this; }
		const UIElement* AsUIElement() const override { return this; }
		UINavigable* AsUINavigable() override { return this; }
		const UINavigable* AsUINavigable() const override { return this; }

		bool CanFocus() const override
		{
			if (!m_focusable)
				return false;
			if (!GetIsActive())
				return false;

			auto owner = GetOwner();
			return owner && owner->GetIsActiveInHierarchy();
		}

		bool IsFocused() const override { return m_focused; }

		void Focus() override
		{
			if (!CanFocus())
				return;
			if (m_focused)
				return;

			m_focused = true;
			OnFocused();
		}

		void Blur() override
		{
			if (!m_focused)
				return;

			m_focused = false;
			OnBlurred();
		}

		UUID GetNeighborEntity(UINavigationDirection dir) const override
		{
			return m_neighbors[static_cast<size_t>(dir)];
		}

		void SetNeighborEntity(UINavigationDirection dir, const UUID& neighborEntity) override
		{
			m_neighbors[static_cast<size_t>(dir)] = neighborEntity;
		}

		void Submit() override {}
		void Back() override {}

		bool IsFocusable() const { return m_focusable; }
		void SetFocusable(bool focusable) { m_focusable = focusable; }

	protected:
		virtual void OnFocused() {}
		virtual void OnBlurred() {}

	private:
		bool m_focusable = false;
		bool m_focused = false;
		std::array<UUID, 4> m_neighbors{ UUID::Invalid, UUID::Invalid, UUID::Invalid, UUID::Invalid };
	};
}
