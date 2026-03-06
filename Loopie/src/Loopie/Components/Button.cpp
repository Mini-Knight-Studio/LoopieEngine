#include "Button.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Scene/Scene.h"
#include "Loopie/Scripting/ScriptingManager.h"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

namespace Loopie
{
	void Button::Init()
	{
		if (!GetOwner())
			return;

		if (!GetOwner()->HasComponent<RectTransform>())
			GetOwner()->ReplaceTransform<RectTransform>();

		ApplyState(m_interactable ? VisualState::Normal : VisualState::Disabled);
	}

	void Button::SetInteractable(bool v)
	{
		m_interactable = v;

		if (!m_interactable)
		{
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
		if (!ScriptingManager::IsRunning())
			return;

		m_isHovered = hovered;

		if (!m_interactable)
		{
			ApplyState(VisualState::Disabled);
			return;
		}

		if (m_isPressed)
			ApplyState(VisualState::Pressed);
		else
			ApplyState(m_isHovered ? VisualState::Hovered : VisualState::Normal);
	}

	void Button::SetPressed(bool pressed)
	{
		if (!ScriptingManager::IsRunning())
			return;

		m_isPressed = pressed;

		if (!m_interactable)
		{
			ApplyState(VisualState::Disabled);
			return;
		}

		if (m_isPressed)
			ApplyState(VisualState::Pressed);
		else
			ApplyState(m_isHovered ? VisualState::Hovered : VisualState::Normal);
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

	void Button::ApplyState(VisualState state)
	{
		if (m_currentState == state)
			return;

		m_currentState = state;

		Image* img = GetOwner() ? GetOwner()->GetComponent<Image>() : nullptr;
		if (!img || !img->GetIsActive())
			return;

		switch (state)
		{
		case VisualState::Normal: img->SetTint(m_normalColor); break;
		case VisualState::Hovered: img->SetTint(m_hoveredColor); break;
		case VisualState::Pressed: img->SetTint(m_pressedColor); break;
		case VisualState::Disabled: img->SetTint(m_disabledColor); break;
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

	JsonNode Button::Serialize(JsonNode& parent) const
	{
		JsonNode node = parent.CreateObjectField("button");

		node.CreateField<bool>("interactable", m_interactable);

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
		m_interactable = data.GetValue<bool>("interactable", true).Result;

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

		ApplyState(m_interactable ? VisualState::Normal : VisualState::Disabled);
	}

	bool Button::IsFunctionCallConfigured(const FunctionCall& functionCall)
	{
		return !functionCall.Function.empty();
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
