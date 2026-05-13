#include "Button.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Scripting/ScriptingManager.h"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <algorithm>
#include <unordered_set>

namespace Loopie
{
	Button::Button()
	{
		SetFocusable(true);
	}

	bool Button::CanFocus() const
	{
		return UIElement::CanFocus() && m_interactable;
	}

	void Button::Submit()
	{
		if (!ScriptingManager::IsRunning())
			return;
		if (!m_interactable)
			return;

		if (m_submitPressDuration > 0.0f)
		{
			SetPressed(true);
			m_submitPressTimer = m_submitPressDuration;
		}

		InvokeOnClick();
	}

	void Button::SetSubmitPressDuration(float seconds)
	{
		m_submitPressDuration = std::max(0.0f, seconds);
		if (m_submitPressTimer > m_submitPressDuration)
			m_submitPressTimer = m_submitPressDuration;
	}

	void Button::OnUpdate()
	{
		if (!ScriptingManager::IsRunning())
			return;

		if (m_submitPressTimer <= 0.0f)
			return;

		m_submitPressTimer -= (float)Time::GetDeltaTime();
		if (m_submitPressTimer <= 0.0f)
		{
			m_submitPressTimer = 0.0f;
			SetPressed(false);
		}
	}

	void Button::Init()
	{
		if (!GetOwner())
			return;

		SetFocusable(true);

		if (!GetOwner()->HasComponent<RectTransform>())
			GetOwner()->ReplaceTransform<RectTransform>();

		ApplyState(m_interactable ? VisualState::Normal : VisualState::Disabled, true);
	}

	void Button::SetTransitionMode(VisualTransitionMode mode)
	{
		if (m_transitionMode == mode)
			return;

		m_transitionMode = mode;
		ApplyState(m_currentState, true);
	}

	void Button::SetNormalTexture(const std::shared_ptr<Texture>& texture)
	{
		m_normalTexture = texture;
		ApplyState(m_currentState, true);
	}

	void Button::SetHoveredTexture(const std::shared_ptr<Texture>& texture)
	{
		m_hoveredTexture = texture;
		ApplyState(m_currentState, true);
	}

	void Button::SetPressedTexture(const std::shared_ptr<Texture>& texture)
	{
		m_pressedTexture = texture;
		ApplyState(m_currentState, true);
	}

	void Button::SetDisabledTexture(const std::shared_ptr<Texture>& texture)
	{
		m_disabledTexture = texture;
		ApplyState(m_currentState, true);
	}

	void Button::SetVisualPropagationMode(VisualPropagationMode mode)
	{
		if (m_visualPropagationMode == mode)
			return;

		m_visualPropagationMode = mode;
		ApplyState(m_currentState, true);
	}

	void Button::SetVisualPropagationTargets(const std::vector<UUID>& targets)
	{
		m_visualPropagationTargets.clear();
		m_visualPropagationTargets.reserve(targets.size());
		std::unordered_set<UUID> seen;
		seen.reserve(targets.size());
		for (const UUID& id : targets)
		{
			if (id == UUID::Invalid)
				continue;
			if (seen.find(id) != seen.end())
				continue;
			seen.insert(id);
			m_visualPropagationTargets.push_back(id);
		}
		ApplyState(m_currentState, true);
	}

	void Button::ApplyExternalVisualState(VisualState state)
	{
		m_externalVisualOverride = true;
		ApplyState(state, true, false);

		if (auto owner = GetOwner())
		{
			if (Text* text = owner->GetComponent<Text>())
			{
				if (text->GetIsActive())
					ApplyStateColor(*text, state);
			}
		}
	}

	void Button::SetInteractable(bool v)
	{
		m_interactable = v;
		m_externalVisualOverride = false;

		if (!m_interactable)
		{
			m_submitPressTimer = 0.0f;
			m_isHovered = false;
			m_isPressed = false;
			ApplyState(VisualState::Disabled);
		}
		else
		{
			ApplyState(m_isPressed ? VisualState::Pressed : (m_isHovered ? VisualState::Hovered : VisualState::Normal));
		}
	}

	void Button::SetHovered(bool hovered)
	{
		if (m_externalVisualOverride)
		{
			const bool direct = hovered || IsFocused() || m_isPressed;
			if (!direct)
				return;
			m_externalVisualOverride = false;
		}

		m_isHovered = hovered;

		const bool applyToComponents = ScriptingManager::IsRunning();

		if (!m_interactable)
		{
			if (applyToComponents)
				ApplyState(VisualState::Disabled);
			else
				m_currentState = VisualState::Disabled;
			return;
		}

		if (m_isPressed)
		{
			if (applyToComponents)
				ApplyState(VisualState::Pressed);
			else
				m_currentState = VisualState::Pressed;
		}
		else
		{
			const VisualState state = m_isHovered ? VisualState::Hovered : VisualState::Normal;
			if (applyToComponents)
				ApplyState(state);
			else
				m_currentState = state;
		}
	}

	void Button::SetPressed(bool pressed)
	{
		if (m_externalVisualOverride)
		{
			const bool direct = pressed || IsFocused() || m_isHovered;
			if (!direct)
				return;
			m_externalVisualOverride = false;
		}

		m_isPressed = pressed;

		const bool applyToComponents = ScriptingManager::IsRunning();

		if (!m_interactable)
		{
			if (applyToComponents)
				ApplyState(VisualState::Disabled);
			else
				m_currentState = VisualState::Disabled;
			return;
		}

		if (m_isPressed)
		{
			if (applyToComponents)
				ApplyState(VisualState::Pressed);
			else
				m_currentState = VisualState::Pressed;
		}
		else
		{
			const VisualState state = m_isHovered ? VisualState::Hovered : VisualState::Normal;
			if (applyToComponents)
				ApplyState(state);
			else
				m_currentState = state;
		}
	}

	void Button::TriggerClick()
	{
		if (!ScriptingManager::IsRunning())
			return;

		if (!m_interactable)
			return;

		InvokeOnClick();
	}

	std::vector<FunctionCall> Button::GetFlattenedOnClickFunctionCalls() const
	{
		std::vector<FunctionCall> functionCalls;
		for (const auto& [entityUUID, calls] : m_onClickFunctionCalls)
		{
			functionCalls.insert(functionCalls.end(), calls.begin(), calls.end());
		}

		return functionCalls;
	}

	void Button::SetOnClickFunctionCalls(const std::vector<FunctionCall>& functionCalls)
	{
		m_onClickFunctionCalls.clear();

		for (const FunctionCall& functionCall : functionCalls)
			AddOnClickFunctionCall(functionCall);
	}

	void Button::AddOnClickFunctionCall(const FunctionCall& functionCall)
	{
		m_onClickFunctionCalls[functionCall.EntityUUID].push_back(functionCall);
	}

	void Button::ApplyState(VisualState state, bool force, bool propagate)
	{
		if (!force && m_currentState == state)
			return;

		m_currentState = state;

		// Apply to this button's own visuals (if it has an Image)
		if (auto owner = GetOwner())
		{
			Image* img = owner->GetComponent<Image>();
			if (img && img->GetIsActive())
			{
				switch (m_transitionMode)
				{
					case VisualTransitionMode::ColorTint:
						ApplyStateTint(*img, state);
						break;

					case VisualTransitionMode::TextureSwap:
						ApplyStateTexture(*img, state);
						break;
				}
			}
		}

		// Apply to propagation targets
		if (propagate)
			ApplyVisualStateToPropagationTargets(state);
	}

	void Button::ApplyVisualStateToEntity(const std::shared_ptr<Entity>& entity, VisualState state) const
	{
		if (!entity)
			return;
		if (!entity->GetIsActiveInHierarchy())
			return;

		if (Button* button = entity->GetComponent<Button>())
		{
			if (button != this)
			{
				button->ApplyExternalVisualState(state);
				return;
			}
		}

		if (Image* img = entity->GetComponent<Image>())
		{
			if (img->GetIsActive())
			{
				switch (m_transitionMode)
				{
					case VisualTransitionMode::ColorTint:
						ApplyStateTint(*img, state);
						break;
					case VisualTransitionMode::TextureSwap:
						ApplyStateTexture(*img, state);
						break;
				}
			}
		}

		if (Text* text = entity->GetComponent<Text>())
		{
			if (text->GetIsActive())
				ApplyStateColor(*text, state);
		}
	}

	void Button::ApplyVisualStateToPropagationTargets(VisualState state) const
	{
		if (m_visualPropagationMode == VisualPropagationMode::None)
			return;

		Scene& scene = Application::GetInstance().GetScene();
		const auto owner = GetOwner();
		const UUID ownerUUID = owner ? owner->GetUUID() : UUID::Invalid;

		if (m_visualPropagationMode == VisualPropagationMode::Children)
		{
			if (!owner)
				return;

			std::vector<std::shared_ptr<Entity>> children;
			owner->GetRecursiveChildren(children);
			for (const auto& child : children)
				ApplyVisualStateToEntity(child, state);
			return;
		}

		if (m_visualPropagationMode == VisualPropagationMode::Targets)
		{
			for (const UUID& targetUUID : m_visualPropagationTargets)
			{
				if (targetUUID == UUID::Invalid)
					continue;
				if (targetUUID == ownerUUID)
					continue;

				ApplyVisualStateToEntity(scene.GetEntity(targetUUID), state);
			}
		}
	}

	void Button::OnFocused()
	{
		m_externalVisualOverride = false;
		m_isHovered = true;
		if (!m_interactable)
		{
			ApplyState(VisualState::Disabled);
		}
		else
		{
			ApplyState(VisualState::Hovered);
		}
	}

	void Button::OnBlurred()
	{
		m_externalVisualOverride = false;
		m_isHovered = false;
		m_isPressed = false;
		if (!m_interactable)
		{
			ApplyState(VisualState::Disabled);
		}
		else
		{
			ApplyState(VisualState::Normal);
		}
	}

	void Button::ApplyStateTint(Image& image, VisualState state) const
	{
		switch (state)
		{
		case VisualState::Normal: image.SetTint(m_normalColor); break;
		case VisualState::Hovered: image.SetTint(m_hoveredColor); break;
		case VisualState::Pressed: image.SetTint(m_pressedColor); break;
		case VisualState::Disabled: image.SetTint(m_disabledColor); break;
		}
	}

	void Button::ApplyStateTexture(Image& image, VisualState state) const
	{
		std::shared_ptr<Texture> tex;
		switch (state)
		{
		case VisualState::Normal: tex = m_normalTexture; break;
		case VisualState::Hovered: tex = m_hoveredTexture; break;
		case VisualState::Pressed: tex = m_pressedTexture; break;
		case VisualState::Disabled: tex = m_disabledTexture; break;
		}

		if (tex)
			image.SetTexture(tex);
	}

	void Button::ApplyStateColor(Text& text, VisualState state) const
	{
		switch (state)
		{
			case VisualState::Normal: text.SetColor(m_normalColor); break;
			case VisualState::Hovered: text.SetColor(m_hoveredColor); break;
			case VisualState::Pressed: text.SetColor(m_pressedColor); break;
			case VisualState::Disabled: text.SetColor(m_disabledColor); break;
		}
	}

	void Button::InvokeOnClick()
	{
		if (!ScriptingManager::IsRunning())
			return;

		if (m_onClickFunctionCalls.empty())
			return;

		for (const auto& [entityUUID, functionCalls] : m_onClickFunctionCalls)
		{
			for (const FunctionCall& functionCall : functionCalls)
			{
				if (!IsFunctionCallConfigured(functionCall))
					continue;

				std::shared_ptr<Entity> targetEntity;
				ScriptClass* target = nullptr;
				if (!TryResolveTarget(functionCall, targetEntity, target))
				{
					Log::Warn("Button OnClick target not found. Entity UUID: '{}' Component UUID: '{}'",
						functionCall.EntityUUID.Get(), functionCall.ComponentUUID.Get());
					continue;
				}

				if (!target->GetScriptingClass())
				{
					Log::Warn("Button OnClick: ScriptClass has no scripting class (class name: '{}') on entity '{}'",
						target->GetClassName(), targetEntity->GetName());
					continue;
				}

				if (!target->GetInstance())
				{
					Log::Warn("Button OnClick: ScriptClass instance is null (class name: '{}') on entity '{}'. Did ScriptClass::SetUp run?",
						target->GetClassName(), targetEntity->GetName());
					continue;
				}

				MonoObject* instance = target->GetInstance();
				MonoClass* klass = mono_object_get_class(instance);
				if (!klass)
					continue;

				MonoMethod* method = mono_class_get_method_from_name(klass, functionCall.Function.c_str(), 0);
				if (!method)
				{
					Log::Warn("Button OnClick method not found: {}() on entity '{}'", functionCall.Function, targetEntity->GetName());
					continue;
				}

				MonoObject* exc = nullptr;
				mono_runtime_invoke(method, instance, nullptr, &exc);
				if (exc)
					Log::Error("Exception invoking Button OnClick: {}() on entity '{}'", functionCall.Function, targetEntity->GetName());
			}
		}
	}

	void Button::GetCurrentColor(vec4& outColor) const 
	{
		if (m_transitionMode != VisualTransitionMode::ColorTint)
			return;

		if (m_currentState == VisualState::Normal)
			outColor = m_normalColor;
		else if (m_currentState == VisualState::Hovered)
			outColor = m_hoveredColor;
		else if (m_currentState == VisualState::Pressed)
			outColor = m_pressedColor;
		else if (m_currentState == VisualState::Disabled)
			outColor = m_disabledColor;
	}

	void Button::GetCurrentTexture(std::shared_ptr<Texture>& outTexture) const
	{
		if (m_transitionMode != VisualTransitionMode::TextureSwap)
			return;

		if (m_currentState == VisualState::Normal)
			outTexture = m_normalTexture;
		else if (m_currentState == VisualState::Hovered)
			outTexture = m_hoveredTexture;
		else if (m_currentState == VisualState::Pressed)
			outTexture = m_pressedTexture;
		else if (m_currentState == VisualState::Disabled)
			outTexture = m_disabledTexture;
	}

	JsonNode Button::Serialize(JsonNode& parent) const
	{
		JsonNode node = parent.CreateObjectField("button");
		SerializeDrawOrder(node);
		SerializeNavigation(node);

		node.CreateField<bool>("interactable", m_interactable);
		node.CreateField<int>("transition_mode", static_cast<int>(m_transitionMode));
		node.CreateField<float>("submit_press_duration", m_submitPressDuration);

		auto writeColor = [&](const char* name, const vec4& c)
		{
			JsonNode obj = node.CreateObjectField(name);
			obj.CreateField<float>("r", c.r);
			obj.CreateField<float>("g", c.g);
			obj.CreateField<float>("b", c.b);
			obj.CreateField<float>("a", c.a);
		};

		writeColor("normal", m_normalColor);
		writeColor("hovered", m_hoveredColor);
		writeColor("pressed", m_pressedColor);
		writeColor("disabled", m_disabledColor);

		auto writeTexture = [&](const char* name, const std::shared_ptr<Texture>& tex)
		{
			if (tex)
				node.CreateField<std::string>(name, tex->GetUUID().Get());
		};

		writeTexture("normal_texture", m_normalTexture);
		writeTexture("hovered_texture", m_hoveredTexture);
		writeTexture("pressed_texture", m_pressedTexture);
		writeTexture("disabled_texture", m_disabledTexture);

		node.CreateField<int>("visual_propagation_mode", static_cast<int>(m_visualPropagationMode));
		JsonNode propagationTargets = node.CreateArrayField("visual_propagation_targets");
		for (const UUID& target : m_visualPropagationTargets)
		{
			if (target == UUID::Invalid)
				continue;
			propagationTargets.AddArrayElement<std::string>(target.Get());
		}

		JsonNode onClickBindings = node.CreateObjectField("on_click_bindings");
		for (const auto& [entityUUID, functionCalls] : m_onClickFunctionCalls)
		{
			if (functionCalls.empty())
				continue;

			JsonNode entityCallsNode = onClickBindings.CreateObjectField(entityUUID.Get());
			for (size_t i = 0; i < functionCalls.size(); ++i)
			{
				const FunctionCall& functionCall = functionCalls[i];

				json callJson = json::object();
				callJson["entity_uuid"] = functionCall.EntityUUID.Get();
				callJson["component_uuid"] = functionCall.ComponentUUID.Get();
				callJson["function"] = functionCall.Function;
				entityCallsNode.CreateField(std::to_string(i), callJson);
			}
		}

		return node;
	}

	void Button::Deserialize(const JsonNode& data)
	{
		DeserializeDrawOrder(data);
		DeserializeNavigation(data);
		m_interactable = data.GetValue<bool>("interactable", true).Result;
		m_transitionMode = static_cast<VisualTransitionMode>(data.GetValue<int>("transition_mode", 0).Result);
		m_submitPressDuration = std::max(0.0f, data.GetValue<float>("submit_press_duration", m_submitPressDuration).Result);
		m_visualPropagationMode = static_cast<VisualPropagationMode>(data.GetValue<int>("visual_propagation_mode", 0).Result);

		auto readColor = [&](const char* name, vec4& out)
		{
			JsonNode obj = data.Child(name);
			if (!obj.IsValid() || !obj.IsObject())
				return;

			out.r = obj.GetValue<float>("r", out.r).Result;
			out.g = obj.GetValue<float>("g", out.g).Result;
			out.b = obj.GetValue<float>("b", out.b).Result;
			out.a = obj.GetValue<float>("a", out.a).Result;
		};

		readColor("normal", m_normalColor);
		readColor("hovered", m_hoveredColor);
		readColor("pressed", m_pressedColor);
		readColor("disabled", m_disabledColor);

		auto readTexture = [&](const char* name, std::shared_ptr<Texture>& out)
		{
			const std::string uuidStr = data.GetValue<std::string>(name, "").Result;
			if (!UUID::IsValid(uuidStr))
			{
				out.reset();
				return;
			}

			Metadata* meta = AssetRegistry::GetMetadata(UUID(uuidStr));
			if (!meta)
			{
				out.reset();
				return;
			}

			out = ResourceManager::GetTexture(*meta);
			if (!out)
				out->Load();
		};

		readTexture("normal_texture", m_normalTexture);
		readTexture("hovered_texture", m_hoveredTexture);
		readTexture("pressed_texture", m_pressedTexture);
		readTexture("disabled_texture", m_disabledTexture);

		m_visualPropagationTargets.clear();
		JsonNode propagationTargets = data.Child("visual_propagation_targets");
		if (propagationTargets.IsValid() && propagationTargets.IsArray())
		{
			const unsigned int n = propagationTargets.Size();
			m_visualPropagationTargets.reserve(n);
			for (unsigned int i = 0; i < n; ++i)
			{
				JsonResult<std::string> uuidStr = propagationTargets.GetArrayElement<std::string>(i);
				if (!uuidStr.Found || !UUID::IsValid(uuidStr.Result))
					continue;
				m_visualPropagationTargets.emplace_back(UUID(uuidStr.Result));
			}
		}

		m_onClickFunctionCalls.clear();

		JsonNode onClickBindings = data.Child("on_click_bindings");
		if (onClickBindings.IsValid() && onClickBindings.IsObject())
		{
			for (const std::string& entityUUIDString : onClickBindings.GetObjectKeys())
			{
				JsonNode entityCallsNode = onClickBindings.Child(entityUUIDString);
				if (!entityCallsNode.IsObject())
					continue;

				for (const std::string& callIndex : entityCallsNode.GetObjectKeys())
				{
					JsonNode callNode = entityCallsNode.Child(callIndex);
					if (!callNode.IsValid())
						continue;

					FunctionCall functionCall{
						UUID(callNode.GetValue<std::string>("entity_uuid", entityUUIDString).Result),
						UUID(callNode.GetValue<std::string>("component_uuid", "00000000-0000-0000-0000-000000000000").Result),
						callNode.GetValue<std::string>("function", "").Result
					};

					AddOnClickFunctionCall(functionCall);
				}
			}
		}

		if (m_onClickFunctionCalls.empty())
		{
			const std::string oldMethod = data.GetValue<std::string>("on_click_method", "").Result;
			const std::string oldComponentUUID = data.GetValue<std::string>("on_click_script_uuid", "").Result;

			auto owner = GetOwner();
			if (owner && !oldMethod.empty() && UUID::IsValid(oldComponentUUID))
			{
				AddOnClickFunctionCall({ owner->GetUUID(), UUID(oldComponentUUID), oldMethod });
			}
		}

		ApplyState(m_interactable ? VisualState::Normal : VisualState::Disabled, true);
	}

	void Button::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const Button& otherButton = static_cast<const Button&>(other);
		CloneDrawOrder(otherButton);
		CloneNavigation(otherButton);
		m_interactable = otherButton.m_interactable;
		m_transitionMode = otherButton.m_transitionMode;
		m_submitPressDuration = otherButton.m_submitPressDuration;
		m_normalColor = otherButton.m_normalColor;
		m_hoveredColor = otherButton.m_hoveredColor;
		m_pressedColor = otherButton.m_pressedColor;
		m_disabledColor = otherButton.m_disabledColor;
		m_normalTexture = otherButton.m_normalTexture;
		m_hoveredTexture = otherButton.m_hoveredTexture;
		m_pressedTexture = otherButton.m_pressedTexture;
		m_disabledTexture = otherButton.m_disabledTexture;
		m_onClickFunctionCalls = otherButton.m_onClickFunctionCalls;
		m_visualPropagationMode = otherButton.m_visualPropagationMode;
		m_visualPropagationTargets = otherButton.m_visualPropagationTargets;
		ApplyState(m_interactable ? VisualState::Normal : VisualState::Disabled, true);
	}

	bool Button::IsFunctionCallConfigured(const FunctionCall& functionCall)
	{
		return !(functionCall.EntityUUID == UUID::Invalid)
			&& !(functionCall.ComponentUUID == UUID::Invalid)
			&& !functionCall.Function.empty();
	}

	bool Button::TryResolveTarget(const FunctionCall& functionCall, std::shared_ptr<Entity>& entity, ScriptClass*& scriptComponent)
	{
		Scene& scene = Application::GetInstance().GetScene();
		entity = scene.GetEntity(functionCall.EntityUUID);
		if (!entity)
			return false;

		Component* component = entity->GetComponent(functionCall.ComponentUUID);
		if (!component)
			return false;

		if (component->GetTypeID() != ScriptClass::GetTypeIDStatic())
			return false;

		scriptComponent = static_cast<ScriptClass*>(component);
		return scriptComponent != nullptr;
	}
}
