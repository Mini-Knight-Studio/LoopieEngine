#include "InputEventManager.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Core/Time.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Window.h"

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include <cmath>

namespace Loopie {
	InputEventManager::InputEventManager()
	{
		m_keyboard.fill(KeyState::IDLE);
		m_gamepad.fill(KeyState::IDLE);
		m_mouse.fill(KeyState::IDLE);
		m_axesRaw.fill(0.0f);
		m_axesSmoothed.fill(0.0f);

		m_touchedEvents.reserve(15);

		

	}
	InputEventManager::~InputEventManager()
	{
		if (m_gamepadController) {
			SDL_CloseGamepad(m_gamepadController);
			m_gamepadController = nullptr;
		}
	}

	void InputEventManager::Update() {

		for (Uint32 t : m_touchedEvents) {
			m_events[t] = false;
		}
		m_touchedEvents.clear();
		m_droppedFiles.clear();

		AdvanceKeyStates(m_keyboard);
		AdvanceKeyStates(m_gamepad);
		AdvanceKeyStates(m_mouse);

		m_mouseDelta = { 0.0f, 0.0f };
		m_scrollDelta = { 0.0f, 0.0f };

		anyKey = false;
		anyButton = false;
		anyMouseButton = false;
		any = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);

			if (!m_events[event.type]) {
				m_events.set(event.type, true);
				m_touchedEvents.push_back(event.type);
			}

			switch (event.type) {
				case SDL_EVENT_KEY_DOWN:

					if (!event.key.repeat) {
						m_keyboard[event.key.scancode] = KeyState::DOWN;
						m_lastPressedKey = event.key.scancode;
						any = true;
						anyKey = true;

						m_currentDeviceType = InputDevice::MOUSE_KEYBOARD;

					}
					break;

				case SDL_EVENT_KEY_UP:
					m_keyboard[event.key.scancode] = KeyState::UP;
					break;

				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					m_gamepad[event.gbutton.button] = KeyState::DOWN;
					m_lastPressedButton = (SDL_GamepadButton)event.gbutton.button;
					any = true;
					anyButton = true;

					m_currentDeviceType = InputDevice::CONTROLLER;

					break;

				case SDL_EVENT_GAMEPAD_BUTTON_UP:
					m_gamepad[event.gbutton.button] = KeyState::UP;
					break;

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (event.button.button <= m_mouse.size()) {
						m_mouse[event.button.button - 1] = KeyState::DOWN;
						any = true;
						anyMouseButton = true;

						m_currentDeviceType = InputDevice::MOUSE_KEYBOARD;
					}
					break;
				case SDL_EVENT_MOUSE_BUTTON_UP:
					if (event.button.button <= m_mouse.size())
						m_mouse[event.button.button - 1] = KeyState::UP;
					break;

				case SDL_EVENT_MOUSE_MOTION:
					m_mouseDelta = { event.motion.xrel, event.motion.yrel };
					m_mousePosition = { event.motion.x, event.motion.y };

					if(m_mouseDelta != vec2(0,0))
						m_currentDeviceType = InputDevice::MOUSE_KEYBOARD;
					break;

				case SDL_EVENT_MOUSE_WHEEL:
					m_scrollDelta = { event.wheel.x, event.wheel.y };

					if(m_scrollDelta!= vec2(0,0))
						m_currentDeviceType = InputDevice::MOUSE_KEYBOARD;
					break;

				case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
					float value = event.gaxis.value;
					float normalized = value / 32768.0f;

					if (std::abs(normalized) < m_axisDeadZone)
						normalized = 0.0f;

					m_axesRaw[event.gaxis.axis] = normalized;

					if(normalized!=0)
						m_currentDeviceType = InputDevice::CONTROLLER;

					break;
				}

				case SDL_EVENT_GAMEPAD_ADDED: {
					if (m_gamepadController) {
						SDL_CloseGamepad(m_gamepadController);
					}
					m_gamepadController = SDL_OpenGamepad(event.gdevice.which);
					if (m_gamepadController) {
						Log::Info("Gamepad connected: {0}", SDL_GetGamepadName(m_gamepadController));
					}
					break;
				}

				case SDL_EVENT_GAMEPAD_REMOVED: {
					Log::Info("Gamepad removed");
					if (m_gamepadController) {
						SDL_CloseGamepad(m_gamepadController);
						m_gamepadController = nullptr;
					}
					ResetShake();
					break;
				}

				case SDL_EVENT_DROP_FILE: {
					const char* droppedFile = event.drop.data;
					m_droppedFiles.push_back(droppedFile);
					Log::Info("Dropped file: '{0}'", droppedFile);
					break;
				}

				default:
					break;
			}

		}

		float dt = Time::GetDeltaTime();
		float t = 1.0f - std::exp(-m_axisSmoothing * dt);
		for (size_t i = 0; i < SDL_GAMEPAD_AXIS_COUNT; i++)
		{
			m_axesSmoothed[i] += (m_axesRaw[i] - m_axesSmoothed[i]) * t;
		}

		UpdateShake();
	}

	KeyState InputEventManager::GetKeyStatus(SDL_Scancode keyCode) const
	{
		return m_keyboard[keyCode];
	}

	bool InputEventManager::GetKeyWithModifier(SDL_Scancode keyCode, KeyModifier modifier) const
	{
		if (m_keyboard[keyCode] != KeyState::DOWN)
			return false;

		unsigned int leftKey = 0;
		unsigned int rightKey = 0;
		bool leftModifier = false;
		bool rightModifier = false;

		switch (modifier)
		{
			case Loopie::KeyModifier::SHIFT:
				leftKey = SDL_SCANCODE_LSHIFT;
				rightKey = SDL_SCANCODE_RSHIFT;
				break;
			case Loopie::KeyModifier::CTRL:
				leftKey = SDL_SCANCODE_LCTRL;
				rightKey = SDL_SCANCODE_RCTRL;
				break;
			case Loopie::KeyModifier::ALT:
				leftKey = SDL_SCANCODE_LALT;
				rightKey = SDL_SCANCODE_RALT;
				break;
			default:
				return false;
		}

		leftModifier = m_keyboard[leftKey] == KeyState::DOWN || m_keyboard[leftKey] == KeyState::REPEAT;
		rightModifier = m_keyboard[rightKey] == KeyState::DOWN || m_keyboard[rightKey] == KeyState::REPEAT;

		return leftModifier || rightModifier;
	}

	KeyState InputEventManager::GetGamepadButtonStatus(SDL_GamepadButton controlCode) const
	{		
		return m_gamepad[controlCode];
	}

	KeyState InputEventManager::GetMouseButtonStatus(int mouseIndex) const
	{
		return m_mouse[mouseIndex];
	}

	SDL_Scancode InputEventManager::GetLastPressedKey() const
	{
		return m_lastPressedKey;
	}

	SDL_GamepadButton InputEventManager::GetLastPressedButton() const
	{
		return m_lastPressedButton;
	}

	const vec2& InputEventManager::GetMousePosition() const
	{
		return m_mousePosition;
	}

	const vec2& InputEventManager::GetMouseDelta() const
	{
		return m_mouseDelta;
	}

	const vec2& InputEventManager::GetScrollDelta() const
	{
		return m_scrollDelta;
	}

	vec2 InputEventManager::GetLeftAxis() const
	{
		return { m_axesSmoothed[SDL_GAMEPAD_AXIS_LEFTX], m_axesSmoothed[SDL_GAMEPAD_AXIS_LEFTY] };
	}

	vec2 InputEventManager::GetLeftAxisRaw() const
	{
		return { m_axesRaw[SDL_GAMEPAD_AXIS_LEFTX], m_axesRaw[SDL_GAMEPAD_AXIS_LEFTY] };
	}

	vec2 InputEventManager::GetRightAxis() const
	{
		return { m_axesSmoothed[SDL_GAMEPAD_AXIS_RIGHTX], m_axesSmoothed[SDL_GAMEPAD_AXIS_RIGHTY] };
	}

	vec2 InputEventManager::GetRightAxisRaw() const
	{
		return { m_axesRaw[SDL_GAMEPAD_AXIS_RIGHTX], m_axesRaw[SDL_GAMEPAD_AXIS_RIGHTY] };
	}

	float InputEventManager::GetLeftTrigger() const
	{
		return m_axesSmoothed[SDL_GAMEPAD_AXIS_LEFT_TRIGGER];
	}

	float InputEventManager::GetLeftTriggerRaw() const
	{
		return m_axesRaw[SDL_GAMEPAD_AXIS_LEFT_TRIGGER];
	}

	float InputEventManager::GetRightTrigger() const
	{
		return m_axesSmoothed[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER];
	}

	float InputEventManager::GetRightTriggerRaw() const
	{
		return m_axesRaw[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER];
	}

	float InputEventManager::GetAxisValue(SDL_GamepadAxis axis) const
	{
		return m_axesSmoothed[axis];
	}

	float InputEventManager::GetAxisValueRaw(SDL_GamepadAxis axis) const
	{
		return m_axesRaw[axis];
	}

	const std::vector<const char*>& InputEventManager::GetDroppedFiles() const
	{
		return m_droppedFiles;
	}

	// Remember that index starts at 0.
	const char* InputEventManager::GetDroppedFile(int index) const
	{
		if (index >= m_droppedFiles.size() || index < 0)
		{
			Log::Info("Attempted to get a dropped file out of range. \nIndex was {0}, dropped files' size is {1}",
				index, m_droppedFiles.size());
			return "";
		}
		return m_droppedFiles[index];
	}

	bool InputEventManager::HasFileBeenDropped() const
	{
		return !m_droppedFiles.empty();
	}

	void InputEventManager::StartShake(float intensity, float duration)
	{
		if(!m_gamepadController) {
			return;
		}

		intensity = std::clamp(intensity, 0.0f, 1.0f);
		m_targetShakeIntensity = intensity;

		m_currentShakeIntensity = intensity;
		Uint16 rumbleValue = static_cast<Uint16>(intensity * 65535.0f);
		SDL_RumbleGamepad(m_gamepadController, rumbleValue, rumbleValue, 0);

		if (duration > 0)
			m_shakeRemainingTime = static_cast<double>(duration);
		else
			m_shakeRemainingTime = 0.0;
	}

	void InputEventManager::StopShake()
	{
		m_targetShakeIntensity = 0.0f;
		m_shakeRemainingTime = 0.0;

		if (!m_gamepadController)
			return;

		m_currentShakeIntensity = 0.0f;
		SDL_RumbleGamepad(m_gamepadController, 0, 0, 0);
	}

	void InputEventManager::UpdateShake()
	{
		if (!m_gamepadController)
			return;

		float dt = Time::GetDeltaTime();

		if (m_shakeRemainingTime > 0.0) {
			m_shakeRemainingTime -= dt;
			if (m_shakeRemainingTime <= 0.0) {
				StopShake();
			}
		}

		m_currentShakeIntensity = m_targetShakeIntensity;

		Uint16 rumbleValue = static_cast<Uint16>(m_currentShakeIntensity * 65535.0f);
		SDL_RumbleGamepad(m_gamepadController, rumbleValue, rumbleValue, 0);
	}

	void InputEventManager::ResetShake()
	{
		m_shakeRemainingTime = 0.0;
		m_targetShakeIntensity = 0.0f;
		m_currentShakeIntensity = 0.0f;
	}

	void InputEventManager::SetMouseCaptured(bool capture)
	{
		SDL_SetWindowRelativeMouseMode(Application::GetInstance().GetWindow().GetSDLWindow(), capture);
	}

	bool InputEventManager::IsMouseCaptured() const
	{
		return SDL_GetWindowRelativeMouseMode(Application::GetInstance().GetWindow().GetSDLWindow());
	}
	void InputEventManager::StartReadingInputText() const
	{
		SDL_StartTextInput(Application::GetInstance().GetWindow().GetSDLWindow());
		m_readingInputText = true;
	}
	void InputEventManager::StopReadingInputText() const
	{
		SDL_StopTextInput(Application::GetInstance().GetWindow().GetSDLWindow());
		m_readingInputText = false;
	}
	std::string InputEventManager::KeyToString(SDL_Scancode scancode)
	{
		const char* name = SDL_GetScancodeName(scancode);
		return (name && name[0] != '\0') ? name : "Unknown Key";
	}
	std::string InputEventManager::GamepadButtonToString(SDL_GamepadButton button)
	{
		const char* name = SDL_GetGamepadStringForButton(button);
		return (name && name[0] != '\0') ? name : "Unknown Button";
	}
	std::string InputEventManager::MouseButtonToString(int mouseButton)
	{
		switch (mouseButton)
		{
		case SDL_BUTTON_LEFT:   return "Left Mouse Button";
		case SDL_BUTTON_MIDDLE: return "Middle Mouse Button";
		case SDL_BUTTON_RIGHT:  return "Right Mouse Button";
		case SDL_BUTTON_X1:     return "Mouse Button X1";
		case SDL_BUTTON_X2:     return "Mouse Button X2";
		default:                return "Unknown Mouse Button";
		}
	}
	std::string InputEventManager::MouseIndexToString(int mouseIndex)
	{
		return MouseButtonToString(mouseIndex + 1);
	}
}