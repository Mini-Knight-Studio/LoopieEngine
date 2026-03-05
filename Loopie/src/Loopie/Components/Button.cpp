#include "Button.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/ScriptClass.h"
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

		if (!GetOwner()->HasComponent<Image>())
			Log::Warn("Button requires an Image component on the same entity. Add Image to '{}'.", GetOwner()->GetName());

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
		if (!m_interactable)
			return;

		InvokeOnClick();
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

		if (m_onClickMethod.empty())
			return;

		auto owner = GetOwner();
		if (!owner)
			return;

		ScriptClass* target = nullptr;
		if (UUID::IsValid(m_onClickScriptUUID.Get()))
			target = owner->GetComponent<ScriptClass>(m_onClickScriptUUID);
		else
			target = owner->GetComponent<ScriptClass>();

		if (!target || !target->GetScriptingClass() || !target->GetInstance())
			return;

		MonoObject* instance = target->GetInstance();
		MonoClass* klass = mono_object_get_class(instance);
		if (!klass)
			return;

		MonoMethod* method = mono_class_get_method_from_name(klass, m_onClickMethod.c_str(), 0);
		if (!method)
		{
			Log::Warn("Button OnClick method not found: {}()", m_onClickMethod);
			return;
		}

		MonoObject* exc = nullptr;
		mono_runtime_invoke(method, instance, nullptr, &exc);
		if (exc)
			Log::Error("Exception invoking Button OnClick: {}()", m_onClickMethod);
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

		node.CreateField<std::string>("on_click_script_uuid", m_onClickScriptUUID.Get());
		node.CreateField<std::string>("on_click_method", m_onClickMethod);

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

		m_onClickScriptUUID = UUID(data.GetValue<std::string>("on_click_script_uuid", "").Result);
		m_onClickMethod = data.GetValue<std::string>("on_click_method", "").Result;

		ApplyState(m_interactable ? VisualState::Normal : VisualState::Disabled);
	}
}
