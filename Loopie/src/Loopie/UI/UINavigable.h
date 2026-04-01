#pragma once

#include "Loopie/Core/UUID.h"

namespace Loopie
{
	enum class UINavigationDirection
	{
		Up = 0,
		Down,
		Left,
		Right,
	};

	class UINavigable
	{
	public:
		virtual ~UINavigable() = default;

		virtual bool CanFocus() const = 0;
		virtual bool IsFocused() const = 0;
		virtual void Focus() = 0;
		virtual void Blur() = 0;

		virtual UUID GetNeighborEntity(UINavigationDirection dir) const = 0;
		virtual void SetNeighborEntity(UINavigationDirection dir, const UUID& neighborEntity) = 0;

		virtual void Submit() = 0;
		virtual void Back() = 0;
	};
}
