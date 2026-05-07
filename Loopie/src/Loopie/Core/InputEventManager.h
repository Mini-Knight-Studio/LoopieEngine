#pragma once
#include "Loopie/Math/MathTypes.h"

#include <SDL3/SDL_events.h>
#include <bitset>
#include <vector>
#include <array>

namespace Loopie {
	
	enum class KeyState {
		IDLE,
		UP,
		DOWN,
		REPEAT
	};

	enum class KeyModifier {
		SHIFT,
		CTRL,
		ALT
	};

	enum class InputDevice {
		MOUSE_KEYBOARD,
		CONTROLLER
	};

	class InputEventManager {
	public:
		InputEventManager();
		~InputEventManager();

		void Update();
		void Initialize();

		bool HasEvent(SDL_EventType eventType) const { return m_events[eventType]; }

		KeyState GetKeyStatus(SDL_Scancode keyCode) const;
		bool GetKeyWithModifier(SDL_Scancode keyCode, KeyModifier modifier) const;
		KeyState GetGamepadButtonStatus(SDL_GamepadButton controlCode) const;
		KeyState GetMouseButtonStatus(int mouseIndex) const;

		bool AnyKeyDown() const { return  anyKey; }
		bool AnyButtonDown() const { return anyButton; }
		bool AnyMouseButtonDown() const { return anyMouseButton; }
		bool AnyDown() const { return any; }

		SDL_Scancode GetLastPressedKey() const;
		SDL_GamepadButton GetLastPressedButton() const;

		const vec2& GetMousePosition() const;
		const vec2& GetMouseDelta() const;
		const vec2& GetScrollDelta() const;

		vec2 GetLeftAxis() const;
		vec2 GetLeftAxisRaw() const;
		vec2 GetRightAxis() const;
		vec2 GetRightAxisRaw() const;

		float GetLeftTrigger() const;
		float GetLeftTriggerRaw() const;
		float GetRightTrigger() const;
		float GetRightTriggerRaw() const;

		float GetAxisValue(SDL_GamepadAxis axis) const;
		float GetAxisValueRaw(SDL_GamepadAxis axis) const;

		const std::vector<const char*>& GetDroppedFiles() const;
		const char* GetDroppedFile(int index) const;
		bool HasFileBeenDropped() const;

		void SetAxisDeadzone(float value) { m_axisDeadZone = value; }
		float GetAxisDeadzone() { return m_axisDeadZone; }

		void SetMouseCaptured(bool capture);

		bool IsMouseCaptured() const;

		void StartReadingInputText() const;
		void StopReadingInputText() const;	
		bool IsReadingInputText() const { return m_readingInputText; }

		InputDevice GetCurrentDeviceType() const { return m_currentDeviceType; }


		static std::string KeyToString(SDL_Scancode scancode);
		static std::string GamepadButtonToString(SDL_GamepadButton button);
		static std::string MouseButtonToString(int mouseButton);
		static std::string MouseIndexToString(int mouseIndex);

		
	private:

		template <size_t N>
		void AdvanceKeyStates(std::array<KeyState, N>& arr) {
			for (auto& state : arr) {
				switch (state) {
					case KeyState::DOWN: 
						state = KeyState::REPEAT; 
						break;

					case KeyState::UP:   
						state = KeyState::IDLE;   
						break;

					default: 
						break;
				}
			}
		}
		
	private:
		std::bitset<SDL_EVENT_LAST> m_events;
		std::vector<Uint32> m_touchedEvents;


		std::array<KeyState, SDL_SCANCODE_COUNT> m_keyboard;
		std::array<KeyState, SDL_GAMEPAD_BUTTON_COUNT> m_gamepad;
		std::array<float, SDL_GAMEPAD_AXIS_COUNT> m_axesRaw;
		std::array<float, SDL_GAMEPAD_AXIS_COUNT> m_axesSmoothed;
		std::array<KeyState, 5> m_mouse;

		bool anyKey=false;
		bool anyButton = false;
		bool anyMouseButton = false;
		bool any = false;

		vec2 m_mousePosition = { 0.0f, 0.0f };
		vec2 m_mouseDelta = { 0.0f, 0.0f };
		vec2 m_scrollDelta = { 0.0f, 0.0f };

		float m_axisDeadZone = 0.15f;
		float m_axisSmoothing = 64.0f;

		std::vector<const char*> m_droppedFiles;

		SDL_Gamepad* m_gamepadController = nullptr;

		mutable bool m_readingInputText = false;	

		SDL_Scancode m_lastPressedKey = SDL_SCANCODE_UNKNOWN;
		SDL_GamepadButton m_lastPressedButton = SDL_GAMEPAD_BUTTON_INVALID;

		InputDevice m_currentDeviceType = InputDevice::MOUSE_KEYBOARD;
	};
}