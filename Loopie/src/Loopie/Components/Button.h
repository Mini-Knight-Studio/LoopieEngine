#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Core/UUID.h"
#include "Loopie/Math/MathTypes.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace Loopie
{
	class Entity;
	class ScriptClass;
	class Texture;

	struct FunctionCall
	{
		UUID EntityUUID;
		UUID ComponentUUID;
		std::string Function;
	};

	class Button : public Component
	{
	public:
		
		enum class VisualTransitionMode
		{
			ColorTint,
			TextureSwap,
		};

		enum class VisualState
		{
			Normal,
			Hovered,
			Pressed,
			Disabled
		};

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

		void GetCurrentColor(vec4& outColor) const;
		void GetCurrentTexture(std::shared_ptr<Texture>& outTexture) const;

		VisualTransitionMode GetTransitionMode() const { return m_transitionMode; }
		void SetTransitionMode(VisualTransitionMode mode);

		std::shared_ptr<Texture> GetNormalTexture() const { return m_normalTexture; }
		std::shared_ptr<Texture> GetHoveredTexture() const { return m_hoveredTexture; }
		std::shared_ptr<Texture> GetPressedTexture() const { return m_pressedTexture; }
		std::shared_ptr<Texture> GetDisabledTexture() const { return m_disabledTexture; }

		void SetNormalTexture(const std::shared_ptr<Texture>& texture);
		void SetHoveredTexture(const std::shared_ptr<Texture>& texture);
		void SetPressedTexture(const std::shared_ptr<Texture>& texture);
		void SetDisabledTexture(const std::shared_ptr<Texture>& texture);

		const std::unordered_map<UUID, std::vector<FunctionCall>>& GetOnClickFunctionCalls() const { return m_onClickFunctionCalls; }
		std::vector<FunctionCall> GetFlattenedOnClickFunctionCalls() const;
		void SetOnClickFunctionCalls(const std::vector<FunctionCall>& functionCalls);
		void AddOnClickFunctionCall(const FunctionCall& functionCall);

	private:

		static bool IsFunctionCallConfigured(const FunctionCall& functionCall);
		static bool TryResolveTarget(const FunctionCall& functionCall, std::shared_ptr<Entity>& entity, ScriptClass*& scriptComponent);

		void ApplyState(VisualState state);
		void InvokeOnClick();

		void ApplyStateTint(class Image& image, VisualState state) const;
		void ApplyStateTexture(class Image& image, VisualState state) const;

	private:
		bool m_interactable = true;

		vec4 m_normalColor = vec4(1.0f);
		vec4 m_hoveredColor = vec4(0.8f);
		vec4 m_pressedColor = vec4(0.6f);
		vec4 m_disabledColor = vec4(0.5f);

		std::shared_ptr<Texture> m_normalTexture;
		std::shared_ptr<Texture> m_hoveredTexture;
		std::shared_ptr<Texture> m_pressedTexture;
		std::shared_ptr<Texture> m_disabledTexture;

		std::unordered_map<UUID, std::vector<FunctionCall>> m_onClickFunctionCalls;

		bool m_isHovered = false;
		bool m_isPressed = false;
		VisualState m_currentState = VisualState::Normal;
		VisualTransitionMode m_transitionMode = VisualTransitionMode::ColorTint;
	};
}