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

		int GetSortingLayer() const { return m_sortingLayer; }
		void SetSortingLayer(int layer) { m_sortingLayer = layer; }

		int GetOrderInLayer() const { return m_orderInLayer; }
		void SetOrderInLayer(int order) { m_orderInLayer = order; }

	protected:
		virtual void OnFocused() {}
		virtual void OnBlurred() {}

		void SerializeDrawOrder(JsonNode& node) const
		{
			node.CreateField<int>("sorting_layer", m_sortingLayer);
			node.CreateField<int>("order_in_layer", m_orderInLayer);
		}

		void DeserializeDrawOrder(const JsonNode& node)
		{
			m_sortingLayer = node.GetValue<int>("sorting_layer", 0).Result;
			m_orderInLayer = node.GetValue<int>("order_in_layer", 0).Result;
		}

		void CloneDrawOrder(const UIElement& other)
		{
			m_sortingLayer = other.m_sortingLayer;
			m_orderInLayer = other.m_orderInLayer;
		}

		void SerializeNavigation(JsonNode& node) const
		{
			node.CreateField<std::string>("nav_up", m_neighbors[static_cast<size_t>(UINavigationDirection::Up)].Get());
			node.CreateField<std::string>("nav_down", m_neighbors[static_cast<size_t>(UINavigationDirection::Down)].Get());
			node.CreateField<std::string>("nav_left", m_neighbors[static_cast<size_t>(UINavigationDirection::Left)].Get());
			node.CreateField<std::string>("nav_right", m_neighbors[static_cast<size_t>(UINavigationDirection::Right)].Get());
		}

		void DeserializeNavigation(const JsonNode& node)
		{
			auto readUUID = [&](const char* field, UINavigationDirection dir)
			{
				const std::string id = node.GetValue<std::string>(field, "").Result;
				m_neighbors[static_cast<size_t>(dir)] = id.empty() ? UUID::Invalid : UUID(id);
			};

			readUUID("nav_up", UINavigationDirection::Up);
			readUUID("nav_down", UINavigationDirection::Down);
			readUUID("nav_left", UINavigationDirection::Left);
			readUUID("nav_right", UINavigationDirection::Right);
		}

		void CloneNavigation(const UIElement& other)
		{
			m_neighbors = other.m_neighbors;
		}

	private:
		bool m_focusable = false;
		bool m_focused = false;
		int m_sortingLayer = 0;
		int m_orderInLayer = 0;
		std::array<UUID, 4> m_neighbors{ UUID::Invalid, UUID::Invalid, UUID::Invalid, UUID::Invalid };
	};
}
