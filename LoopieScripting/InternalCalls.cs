using System;
using System.Globalization;
using System.Runtime.CompilerServices;

namespace Loopie
{
    public static class InternalCalls
    {
        #region Log
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Int(int parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Float(float parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector2(Vector2 parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector3(Vector3 parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector4(Vector4 parameter, int mode);
        #endregion
        #region Transform
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetPosition(string entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetPosition(string entityID, Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalPosition(string entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalPosition(string entityID, Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetRotation(string entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetRotation(string entityID, Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalRotation(string entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalRotation(string entityID, Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalScale(string entityID, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalScale(string entityID, Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Translate(string entityID, Vector3 translation, Transform.Space objectSpace);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Rotate(string entityID, Vector3 eulerAngles, Transform.Space objectSpace);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_LookAt(string entityID, Vector3 target, Vector3 worldUp);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Forward(string entityID, out Vector3 forward);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Back(string entityID, out Vector3 back);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Up(string entityID, out Vector3 up);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Down(string entityID, out Vector3 down);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Left(string entityID, out Vector3 left);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Right(string entityID, out Vector3 right);
        #endregion
        #region BoxCollider
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_GetLocalCenter(string entityID, string componentID, out Vector3 center);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetLocalCenter(string entityID, string componentID, Vector3 center);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_GetLocalExtents(string entityID, string componentID, out Vector3 extends);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetLocalExtents(string entityID, string componentID, Vector3 extents);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider_IsColliding(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider_HasCollided(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider_HasEndedCollision(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string BoxCollider_GetLayer(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetLayer(string entityID, string componentID, string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider_IsStatic(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetStatic(string entityID, string componentID, bool isStatic);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider_IsTrigger(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetTrigger(string entityID, string componentID, bool isTrigger);
        #endregion
        #region Animator
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Stop(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_PlayClip(string entityID, string componentID, string clipName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Play(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Pause(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Resume(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Animator_GetCurrentClipIndex(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Animator_GetCurrentClipName(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Animator_GetClipName(string entityID, string componentID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetPlaybackSpeed(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_SetPlaybackSpeed(string entityID, string componentID, float time);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_SetLooping(string entityID, string componentID, bool isLooping);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Animator_IsLooping(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Animator_IsPlaying(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetCurrentTime(string entityID, string componentID);
        #endregion
        #region AudioSource
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_Play(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_Stop(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_SetLoop(string entityID, string componentID, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_SetPitch(string entityID, string componentID, float pitch);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_SetVolume(string entityID, string componentID, float volume);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_SetPan(string entityID, string componentID, float pan);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_SetSet3DMinMaxDistance(string entityID, string componentID, float min, float max);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AudioSource_IsLooping(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float AudioSource_GetPitch(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float AudioSource_GetVolume(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float AudioSource_GetPan(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_GetSet3DMinMaxDistance(string id, string componentID, out float min, out float max);
        #endregion
        #region Camera
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetFov(string entityID, string componentID, float fov);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Camera_GetFov(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetNearPlane(string entityID, string componentID, float fov);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Camera_GetNearPlane(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetFarPlane(string entityID, string componentID, float fov);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Camera_GetFarPlane(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Camera_IsMainCamera(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetMainCamera(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Camera_GetMainCamera();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_GetViewport(string entityID, string componentID, out Vector4 viewport);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetOrthoSize(string entityID, string componentID, float size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Camera_GetOrthoSize(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetProjection(string entityID, string componentID, int projection);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Camera_GetProjection(string entityID, string componentID);
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
        internal extern static bool Entity_AddComponent(string entityID, string componentFullName, out string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(string entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_GetComponent(string entityID, Type componentType, out string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_FindEntityByID(string entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetActive(string entityID, bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsActive(string entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsActiveInHierarchy(string entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetParent(string entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetParent(string entityID, string parentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetName(string entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetName(string entityID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int Entity_GetChildCount(string entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetChild(string entityID, int index);
        #endregion
        #region Component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Component_SetActive(string entityID, string componentID, bool isActive);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Component_IsActive(string entityID, string componentID);
        #endregion
        #region Collisions
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Collisions_Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit, int layerMask);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Collisions_GetLayerBit(string layerName);
        #endregion
        #region SceneManager
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Scene_LoadByID(string sceneID);
        #endregion
        #region Gizmo
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Gizmo_DrawLine(Vector3 start, Vector3 end, Vector4 color);
        #endregion
    }
}