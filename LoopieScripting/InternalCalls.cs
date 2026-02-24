using System;
using System.Runtime.CompilerServices;

namespace Loopie
{
    public static class InternalCalls
    {
        #region Log
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Int(int parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Float(float parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector2(Vector2 parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector3(Vector3 parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector4(Vector4 parameter);
        #endregion
        #region Transform
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_GetPosition(string id, out Vector3 position);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_SetPosition(string id, Vector3 position);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_GetLocalPosition(string id, out Vector3 position);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_SetLocalPosition(string id, Vector3 position);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_GetRotation(string id, out Vector3 rotation);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_SetRotation(string id, Vector3 rotation);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_GetLocalRotation(string id, out Vector3 rotation);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_SetLocalRotation(string id, Vector3 rotation);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_GetLocalScale(string id, out Vector3 scale);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_SetLocalScale(string id, Vector3 scale);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Translate(string id, Vector3 translation, Transform.Space objectSpace);
            
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Rotate(string id, Vector3 eulerAngles, Transform.Space objectSpace);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_LookAt(string id, Vector3 target, Vector3 worldUp);

            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Forward(string id, out Vector3 forward);
            
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Back(string id, out Vector3 back);
            
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Up(string id, out Vector3 up);
            
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Down(string id, out Vector3 down);
            
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Left(string id, out Vector3 left);
            
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            internal extern static void Transform_Right(string id, out Vector3 right);
        #endregion
        #region Animator
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Stop(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Play(string entityID, string clipName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Animator_GetCurrentClipIndex(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Animator_GetCurrentClipName(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Animator_GetClipName(string entityID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetPlaybackSpeed(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_SetPlaybackSpeed(string entityID, float time);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_SetLooping(string entityID, bool isLooping);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Animator_IsLooping(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Animator_IsPlaying(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetCurrentTime(string entityID);
        #endregion
        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyUp(KeyCode key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyPressed(KeyCode key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonDown(MouseButton mouseButton);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonUp(MouseButton mouseButton);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonPressed(MouseButton mouseButton);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsGamepadButtonDown(GamepadButton gamepadButton);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsGamepadButtonUp(GamepadButton gamepadButton);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsGamepadButtonPressed(GamepadButton gamepadButton);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 mousePosition);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMouseDelta(out Vector2 mouseDelta);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetScrollDelta(out Vector2 scrollDelta);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetLeftAxis(out Vector2 axis);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetRightAxis(out Vector2 axis);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsAnyKeyDown();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsAnyButtonDown();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsAnyMouseButtonDown();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsAnyDown();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_SetAxisDeadzone(float deadzone);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Input_GetAxisDeadzone();

        #endregion
        #region Time
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetFixedDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetTimeScale();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_SetTimeScale(float scale);
        #endregion
        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_GetScriptInstance(string entityID, string componentFullName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_Create(string entityName, string parentId);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_Clone(string entityID, bool cloneChilds);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_Destroy(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_AddComponent(string entityID, string componentFullName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(string entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_FindEntityByName(string name);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_FindEntityByID(string entityID);
        #endregion
    }
}