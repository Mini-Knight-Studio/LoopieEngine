using System;
using System.ComponentModel;

namespace Loopie
{
    public class Camera : Component
    {

        public enum ProjectionType
        {
            Perspective = 0,
            Orthographic = 1
        }

        public void SetFov(float fov)
        {
            InternalCalls.Camera_SetFov(entity.ID, ID, fov);
        }

        public float GetFov()
        {
            return InternalCalls.Camera_GetFov(entity.ID, ID);
        }

        public void SetNearPlane(float nearPlane)
        {
            InternalCalls.Camera_SetNearPlane(entity.ID, ID, nearPlane);
        }

        public float GetNearPlane()
        {
            return InternalCalls.Camera_GetNearPlane(entity.ID, ID);
        }

        public void SetFarPlane(float farPlane)
        {
            InternalCalls.Camera_SetFarPlane(entity.ID, ID, farPlane);
        }

        public float GetFarPlane()
        {
            return InternalCalls.Camera_GetFarPlane(entity.ID, ID);
        }

        public bool IsMainCamera()
        {
            return InternalCalls.Camera_IsMainCamera(entity.ID, ID);
        }
      
        public static Camera Main { get { return GetMainCamera(); } set { SetMainCamera(value); } }

        private static void SetMainCamera(Camera camera)
        {
            InternalCalls.Camera_SetMainCamera(camera.entity.ID, camera.ID);
        }

        public void SetAsMainCamera()
        {
            InternalCalls.Camera_SetMainCamera(entity.ID, ID);
        }

        private static Camera GetMainCamera()
        {
            string cameraEntityID = InternalCalls.Camera_GetMainCamera();
            if (cameraEntityID == "")
                return null;

            Entity cameraEntity = new Entity(cameraEntityID);
            return cameraEntity.GetComponent<Camera>();
        }

        public Vector4 GetViewport()
        {
            Vector4 viewport = Vector4.Zero;
            InternalCalls.Camera_GetViewport(entity.ID, ID, out viewport);
            return viewport;
        }

        public void SetOrthoSize(float size)
        {
            InternalCalls.Camera_SetOrthoSize(entity.ID, ID, size);
        }

        public float GetOrthoSize()
        {
            return InternalCalls.Camera_GetOrthoSize(entity.ID, ID);
        }

        public void SetProjection(ProjectionType projection)
        {
            InternalCalls.Camera_SetProjection(entity.ID, ID, (int)projection);
        }

        public ProjectionType GetProjection()
        {
            return (ProjectionType)InternalCalls.Camera_GetProjection(entity.ID, ID);
        }
    }
}
