using System.Collections.Specialized;

namespace Loopie
{
    public class Input
    {
        public static bool IsKeyDown(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyDown(keycode);
        }

        public static bool IsKeyUp(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyUp(keycode);
        }

        public static bool IsKeyPressed(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyPressed(keycode);
        }

        public static bool IsMouseButtonDown(MouseButton mouseButton)
        {
            return InternalCalls.Input_IsMouseButtonDown(mouseButton);
        }

        public static bool IsMouseButtonUp(MouseButton mouseButton)
        {
            return InternalCalls.Input_IsMouseButtonUp(mouseButton);
        }

        public static bool IsMouseButtonPressed(MouseButton mouseButton)
        {
            return InternalCalls.Input_IsMouseButtonPressed(mouseButton);
        }

        public static bool IsGamepadButtonDown(GamepadButton gamepadButton)
        {
            return InternalCalls.Input_IsGamepadButtonDown(gamepadButton);
        }

        public static bool IsGamepadButtonUp(GamepadButton gamepadButton)
        {
            return InternalCalls.Input_IsGamepadButtonUp(gamepadButton);
        }

        public static bool IsGamepadButtonPressed(GamepadButton gamepadButton)
        {
            return InternalCalls.Input_IsGamepadButtonPressed(gamepadButton);
        }
        public static Vector2 MousePosition
        {get { return GetMousePosition(); }}
        public static Vector2 MouseDelta
        {get { return GetMouseDelta(); } }
        public static Vector2 ScrollDelta
        {get { return GetScrollDelta(); } }

        private static Vector2 GetMousePosition()
        {
            Vector2 mousePosition = Vector2.Zero;
            InternalCalls.Input_GetMousePosition(out mousePosition);
            return mousePosition;
        }
        private static Vector2 GetMouseDelta()
        {
            Vector2 mouseDelta = Vector2.Zero;
            InternalCalls.Input_GetMouseDelta(out mouseDelta);
            return mouseDelta;
        }
        private static Vector2 GetScrollDelta()
        {
            Vector2 scrollDelta = Vector2.Zero;
            InternalCalls.Input_GetScrollDelta(out scrollDelta);
            return scrollDelta;
        }

        public static bool AnyKey
        { get { return IsAnyKeyDown(); } }
        public static bool AnyButton
        { get { return IsAnyButtonDown(); } }
        public static bool AnyMouseButton
        { get { return IsAnyMouseButtonDown(); } }
        public static bool Any
        { get { return IsAnyDown(); } }

        private static bool IsAnyKeyDown()
        {
            return InternalCalls.Input_IsAnyKeyDown();
        }
        private static bool IsAnyButtonDown()
        {
            return InternalCalls.Input_IsAnyButtonDown();
        }
        private static bool IsAnyMouseButtonDown()
        {
            return InternalCalls.Input_IsAnyMouseButtonDown();
        }
        private static bool IsAnyDown()
        {
            return InternalCalls.Input_IsAnyDown();
        }

        public static Vector2 LeftAxis
        { get { return GetLeftAxis(); } }

        public static Vector2 RightAxis
        { get { return GetRightAxis(); } }

        private static Vector2 GetLeftAxis()
        {
            Vector2 axis = Vector2.Zero;
            InternalCalls.Input_GetLeftAxis(out axis);
            axis.y *= -1;
            return axis;
        }
        private static Vector2 GetRightAxis()
        {
            Vector2 axis = Vector2.Zero;
            InternalCalls.Input_GetRightAxis(out axis);
            axis.y *= -1;
            return axis;
        }

        public static float AxisDeadzone
        { get { return GetAxisDeadzone(); } set { SetAxisDeadzone(value); } }

        private static void SetAxisDeadzone(float deadzone)
        {       
            InternalCalls.Input_SetAxisDeadzone(deadzone);
        }
        private static float GetAxisDeadzone()
        {
            return InternalCalls.Input_GetAxisDeadzone();
        }
    }
}