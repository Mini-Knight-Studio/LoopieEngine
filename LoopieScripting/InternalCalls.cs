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
        internal extern static void NativeLog_Vector2(ref Vector2 parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector3(ref Vector3 parameter, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector4(ref Vector4 parameter, int mode);
        #endregion
        #region Transform
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetPosition(string entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetPosition(string entityID, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalPosition(string entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalPosition(string entityID, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetRotation(string entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetRotation(string entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalRotation(string entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalRotation(string entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalScale(string entityID, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalScale(string entityID, ref Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Translate(string entityID, ref Vector3 translation, Transform.Space objectSpace);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_Rotate(string entityID, ref Vector3 eulerAngles, Transform.Space objectSpace);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_LookAt(string entityID, ref Vector3 target, ref Vector3 worldUp);

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
        #region Transform
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_GetAnchoredPosition(string entityID, out Vector2 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetAnchoredPosition(string entityID, ref Vector2 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_GetPivot(string entityID, out Vector2 pivot);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetPivot(string entityID, ref Vector2 pivot);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_GetSize(string entityID, out Vector2 size);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetSize(string entityID, ref Vector2 size);
        #endregion
        #region BoxCollider
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_GetLocalCenter(string entityID, string componentID, out Vector3 center);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetLocalCenter(string entityID, string componentID, ref Vector3 center);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_GetLocalExtents(string entityID, string componentID, out Vector3 extends);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetLocalExtents(string entityID, string componentID, ref Vector3 extents);

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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetIncludeMask(string entityID, string componentID, int includeMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int BoxCollider_GetIncludeMask(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_AddIncludeMask(string entityID, string componentID, int includeMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_RemoveIncludeMask(string entityID, string componentID, int includeMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_SetExcludeMask(string entityID, string componentID, int excludeMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int BoxCollider_GetExcludeMask(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_AddExcludeMask(string entityID, string componentID, int excludeMask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider_RemoveExcludeMask(string entityID, string componentID, int excludeMask);
        #endregion
        #region Animator
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_Stop(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Animator_PlayClip(string entityID, string componentID, string clipName, float transitionTime);
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
        internal extern static float Animator_GetCurrentClipDuration(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Animator_GetNextClipIndex(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Animator_GetNextClipName(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetNextClipDuration(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Animator_GetClipName(string entityID, string componentID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Animator_GetClipIndex(string entityID, string componentID, string clipName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetClipDurationByIndex(string entityID, string componentID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetClipDurationByName(string entityID, string componentID, string clipName);
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
        internal extern static bool Animator_IsInTransition(string entityID, string componentID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Animator_GetCurrentTime(string entityID, string componentID);
        #endregion
        #region SpriteAnimator
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string SpriteAnimator_GetTextureUUID(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetTextureUUID(string entityID, string componentID, string textureUUID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_GetGrid(string entityID, string componentID, out int columns, out int rows);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetGrid(string entityID, string componentID, int columns, int rows);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int SpriteAnimator_GetStartFrame(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetStartFrame(string entityID, string componentID, int frame);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int SpriteAnimator_GetFrameCount(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetFrameCount(string entityID, string componentID, int count);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SpriteAnimator_GetFPS(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetFPS(string entityID, string componentID, float fps);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SpriteAnimator_GetLoop(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetLoop(string entityID, string componentID, bool loop);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SpriteAnimator_GetPlayOnStart(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetPlayOnStart(string entityID, string componentID, bool playOnStart);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SpriteAnimator_GetPlaying(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetPlaying(string entityID, string componentID, bool playing);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_Play(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_Stop(string entityID, string componentID, bool resetTime);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int SpriteAnimator_GetCurrentFrame(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetCurrentFrame(string entityID, string componentID, int frameIndex);
        #endregion
        #region AudioSource
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_Play(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_Stop(string entityID, string componentID, float fadeTime);

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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSource_TransitionTo(string id, string componentID, string audioID, float fadeOut, float fadeIn, bool crossFade);
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Camera_ScreenToWorldRay(string entityID, string componentID, ref Vector2 mousePos, out Ray outRay);
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
        internal extern static void Input_GetLeftAxisRaw(out Vector2 axis);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetRightAxis(out Vector2 axis);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetRightAxisRaw(out Vector2 axis);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Input_GetLeftTrigger();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Input_GetLeftTriggerRaw();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Input_GetRightTrigger();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Input_GetRightTriggerRaw();

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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Input_GetCurrentDeviceType();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_StartShake(float intensity, float duration);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_StopShake();

        #endregion
        #region Time
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetFixedDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetUnscaledDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetTimeScale();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_SetTimeScale(float scale);
        #endregion
        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_GetScriptInstance(string entityID, string componentFullName, int index);

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
        internal extern static bool Entity_GetComponent(string entityID, Type componentType, int index, out string componentID);

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

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetChildByName(string entityID, string entityName, bool deepSearch);
        #endregion
        #region Component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Component_SetActive(string entityID, string componentID, bool isActive);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Component_IsActive(string entityID, string componentID);
        #endregion
        #region Collisions
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Collisions_Raycast(ref Vector3 origin, ref Vector3 direction, float maxDistance, out RaycastHit hit, int layerMask);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Collisions_RaycastWithColliderAvoidance(ref Vector3 origin, ref Vector3 direction, float maxDistance, out RaycastHit hit, int layerMask, string entityToAvoidID, string componentToAvoidID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Collisions_RaycastWithEntityAvoidance(ref Vector3 origin, ref Vector3 direction, float maxDistance, out RaycastHit hit, int layerMask, string entityToAvoidID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Collisions_GetLayerBit(string layerName);
        #endregion
        #region SceneManager
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Scene_LoadByID(string sceneID);
        #endregion
        #region Gizmo
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Gizmo_DrawLine(ref Vector3 start, ref Vector3 end, ref Vector4 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Gizmo_DrawCircle(ref Vector3 center, float radius, ref Vector3 normal, float steps, ref Vector4 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Gizmo_DrawSphere(ref Vector3 center, float radius, float steps, ref Vector4 color);
        #endregion
        #region ParticleSystem
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_Play(string entityID, string componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_Stop(string entityID, string componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool ParticleSystem_IsPlaying(string entityID, string componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int ParticleSystem_GetEmitterIndex(string entityID, string componentID, string emitterName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterState(string entityID, string componentID, int emitterIndex, bool activeState);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool ParticleSystem_GetEmitterState(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterName(string entityID, string componentID, int emitterIndex, string emitterName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string ParticleSystem_GetEmitterName(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterSpawnRate(string entityID, string componentID, int emitterIndex, int spawnRate);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int ParticleSystem_GetEmitterSpawnRate(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterMaxParticles(string entityID, string componentID, int emitterIndex, int maxParticles);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int ParticleSystem_GetEmitterMaxParticles(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPosition(string entityID, string componentID, int emitterIndex, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPosition(string entityID, string componentID, int emitterIndex, out Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPositionOffset(string entityID, string componentID, int emitterIndex, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPositionOffset(string entityID, string componentID, int emitterIndex, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterRotation(string entityID, string componentID, int emitterIndex, ref Vector3 rotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterRotation(string entityID, string componentID, int emitterIndex, out Vector3 rotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterFollowParent(string entityID, string componentID, int emitterIndex, bool follow);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool ParticleSystem_GetEmitterFollowParent(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterLocalVelocity(string entityID, string componentID, int emitterIndex, bool local);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool ParticleSystem_GetEmitterLocalVelocity(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropPosition(string entityID, string componentID, int emitterIndex, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPropPosition(string entityID, string componentID, int emitterIndex, out Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropVelocity(string entityID, string componentID, int emitterIndex, ref Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPropVelocity(string entityID, string componentID, int emitterIndex, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropVelocityVariation(string entityID, string componentID, int emitterIndex, ref Vector3 variation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPropVelocityVariation(string entityID, string componentID, int emitterIndex, out Vector3 variation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropPositionVariation(string entityID, string componentID, int emitterIndex, ref Vector3 variation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPropPositionVariation(string entityID, string componentID, int emitterIndex, out Vector3 variation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropColorBegin(string entityID, string componentID, int emitterIndex, ref Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPropColorBegin(string entityID, string componentID, int emitterIndex, out Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropColorEnd(string entityID, string componentID, int emitterIndex, ref Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmitterPropColorEnd(string entityID, string componentID, int emitterIndex, out Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropSizeBegin(string entityID, string componentID, int emitterIndex, float size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float ParticleSystem_GetEmitterPropSizeBegin(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropSizeEnd(string entityID, string componentID, int emitterIndex, float size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float ParticleSystem_GetEmitterPropSizeEnd(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterSizeVariation(string entityID, string componentID, int emitterIndex, float variation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float ParticleSystem_GetEmitterSizeVariation(string entityID, string componentID, int emitterIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmitterPropLifetime(string entityID, string componentID, int emitterIndex, float lifetime);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float ParticleSystem_GetEmitterPropLifetime(string entityID, string componentID, int emitterIndex);
        #endregion

        #region UIElement
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int UIElement_GetSortingLayer(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIElement_SetSortingLayer(string entityID, string componentID, int layer);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int UIElement_GetOrderInLayer(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIElement_SetOrderInLayer(string entityID, string componentID, int order);
        #endregion

        #region UIManager
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string UIManager_GetSelectedEntity(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIManager_SetSelectedEntity(string entityID, string componentID, string selectedEntityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool UIManager_GetBlockNavigation(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void UIManager_SetBlockNavigation(string entityID, string componentID, bool blocked);
        #endregion

        #region Image
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Image_GetTint(string entityID, string componentID, out Vector4 tint);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Image_SetTint(string entityID, string componentID, ref Vector4 tint);
        #endregion
        #region Text
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Text_GetColor(string entityID, string componentID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Text_SetColor(string entityID, string componentID, ref Vector4 color);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Text_GetText(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Text_SetText(string entityID, string componentID, string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Text_SetVisibleCharacters(string entityID, string componentID, int charsVisible);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Text_GetVisibleCharacters(string entityID, string componentID);
        #endregion
        #region Button
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Button_IsInteractable(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetInteractable(string entityID, string componentID, bool interactable);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Button_IsHovered(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Button_IsPressed(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_TriggerClick(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Button_GetTransitionMode(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetTransitionMode(string entityID, string componentID, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_GetNormalColor(string entityID, string componentID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetNormalColor(string entityID, string componentID, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_GetHoveredColor(string entityID, string componentID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetHoveredColor(string entityID, string componentID, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_GetPressedColor(string entityID, string componentID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetPressedColor(string entityID, string componentID, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_GetDisabledColor(string entityID, string componentID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetDisabledColor(string entityID, string componentID, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Button_GetNormalTextureUUID(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetNormalTextureUUID(string entityID, string componentID, string textureUUID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Button_GetHoveredTextureUUID(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetHoveredTextureUUID(string entityID, string componentID, string textureUUID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Button_GetPressedTextureUUID(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetPressedTextureUUID(string entityID, string componentID, string textureUUID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Button_GetDisabledTextureUUID(string entityID, string componentID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Button_SetDisabledTextureUUID(string entityID, string componentID, string textureUUID);
        #endregion
        #region Window
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetTargetFramerate(int targetFramerate);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Window_GetTargetFramerate();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetVSync(bool enabled);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Window_GetVSync();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetFullscreen(bool fullscreen);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Window_GetFullscreen();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetResizable(bool resizable);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetSize(ref Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_GetSize(out Vector2 size);
        #endregion
        #region Application
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Application_Quit();
        #endregion
        #region AudioMixer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioMixer_SetBusVolume(string busName, float volume);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float AudioMixer_GetBusVolume(string busName);
        #endregion
        #region MeshRenderer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string MeshRenderer_RaycastEntityPick(ref Vector3 origin, ref Vector3 direction, float maxDistance);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_GetInstancedMaterial(string entityID, string componentID, out string resourceID, out int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int MeshRenderer_GetMaterialInt(string entityID, string componentID, string key);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float MeshRenderer_GetMaterialFloat(string entityID, string componentID, string key);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_GetMaterialVector2(string entityID, string componentID, string key, out Vector2 vec2);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_GetMaterialVector3(string entityID, string componentID, string key, out Vector3 vec3);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_GetMaterialVector4(string entityID, string componentID, string key, out Vector4 vec4);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_SetMaterialInt(string entityID, string componentID, string key, int value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_SetMaterialFloat(string entityID, string componentID, string key, float value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_SetMaterialVector2(string entityID, string componentID, string key, ref Vector2 vec2);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_SetMaterialVector3(string entityID, string componentID, string key, ref Vector3 vec3);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_SetMaterialVector4(string entityID, string componentID, string key, ref Vector4 vec4);

        #endregion

        //////////RESOURCE INTERNAL CALLS//////////
        ///

        #region Material
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Material_GetInt(string materialID, string key);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Material_GetFloat(string materialID, string key);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetVector2(string materialID, string key, out Vector2 vec);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetVector3(string materialID, string key, out Vector3 vec);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetVector4(string materialID, string key, out Vector4 vec);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetInt(string materialID, string key, int value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetFloat(string materialID, string key, float value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetVector2(string materialID, string key, ref Vector2 vec);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetVector3(string materialID, string key, ref Vector3 vec);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetVector4(string materialID, string key, ref Vector4 vec);
        #endregion
    }
}