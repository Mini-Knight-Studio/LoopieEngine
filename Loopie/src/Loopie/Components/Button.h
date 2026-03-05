#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Core/UUID.h"
#include "Loopie/Math/MathTypes.h"

#include <string>

namespace Loopie
{
	class Button : public Component
	{
	public:
		DEFINE_TYPE(Button)

		Button() = default;
		~Button() override = default;

		void Init() override;
		void OnUpdate() override {}

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

		bool IsInteractable() const { return m_interactable; }
		void SetInteractable(bool v);

		bool IsHovered() const { return m_isHovered; }
		bool IsPressed() const { return m_isPressed; }

		void SetHovered(bool hovered);
		void SetPressed(bool pressed);
		void TriggerClick();

		const vec4& GetNormalColor() const { return m_normalColor; }
		const vec4& GetHoveredColor() const { return m_hoveredColor; }
		const vec4& GetPressedColor() const { return m_pressedColor; }
		const vec4& GetDisabledColor() const { return m_disabledColor; }

		void SetNormalColor(const vec4& color) { m_normalColor = color; }
		void SetHoveredColor(const vec4& color) { m_hoveredColor = color; }
		void SetPressedColor(const vec4& color) { m_pressedColor = color; }
		void SetDisabledColor(const vec4& color) { m_disabledColor = color; }

		const UUID& GetOnClickScriptUUID() const { return m_onClickScriptUUID; }
		const std::string& GetOnClickMethod() const { return m_onClickMethod; }

		void SetOnClickScriptUUID(const UUID& uuid) { m_onClickScriptUUID = uuid; }
		void SetOnClickMethod(const std::string& method) { m_onClickMethod = method; }

	private:
		enum class VisualState
		{
			Normal,
			Hovered,
			Pressed,
			Disabled
		};

		void ApplyState(VisualState state);
		void InvokeOnClick();

	private:
		bool m_interactable = true;

		vec4 m_normalColor = vec4(1.0f);
		vec4 m_hoveredColor = vec4(0.8f);
		vec4 m_pressedColor = vec4(0.6f);
		vec4 m_disabledColor = vec4(0.5f);

		UUID m_onClickScriptUUID{};
		std::string m_onClickMethod;

		bool m_isHovered = false;
		bool m_isPressed = false;
		VisualState m_currentState = VisualState::Normal;
	};
}