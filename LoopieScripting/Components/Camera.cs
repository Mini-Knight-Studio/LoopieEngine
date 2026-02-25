using System;
using System.ComponentModel;

namespace Loopie
{
    public class Camera : Component
    {
        public void SetFov(float fov)
        {
            InternalCalls.Camera_SetFov(entity.ID, fov);
        }

        public float GetFov()
        {
            return InternalCalls.Camera_GetFov(entity.ID);
        }

        public void SetNearPlane(float nearPlane)
        {
            InternalCalls.Camera_SetNearPlane(entity.ID, nearPlane);
        }

        public float GetNearPlane()
        {
            return InternalCalls.Camera_GetNearPlane(entity.ID);
        }

        public void SetFarPlane(float farPlane)
        {
            InternalCalls.Camera_SetFarPlane(entity.ID, farPlane);
        }

        public float GetFarPlane()
        {
            return InternalCalls.Camera_GetFarPlane(entity.ID);
        }

        public bool IsMainCamera()
        {
            return InternalCalls.Camera_IsMainCamera(entity.ID);
        }
      
        public static Camera Main { get { return GetMainCamera(); } set { SetMainCamera(value); } }

        private static void SetMainCamera(Camera camera)
        {
            InternalCalls.Camera_SetMainCamera(camera.entity.ID);
        }

        public void SetAsMainCamera()
        {
            InternalCalls.Camera_SetMainCamera(entity.ID);
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
            InternalCalls.Camera_GetViewport(entity.ID, out viewport);
            return viewport;
        }

    }
}
