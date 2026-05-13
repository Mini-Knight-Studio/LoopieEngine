using System;

namespace Loopie
{
    public sealed class MeshRenderer : Component
    {
        public Material GetInstancedMaterial()
        {
            InternalCalls.MeshRenderer_GetInstancedMaterial(entity.ID, ID, out string resourceID, out int index);
            return new Material(resourceID, index);
        }

        ////MATERIAL
        ///
        public int GetInt(string key)
        {
            return InternalCalls.MeshRenderer_GetMaterialInt(entity.ID, ID, key);
        }
        public float GetFloat(string key)
        {
            return InternalCalls.MeshRenderer_GetMaterialFloat(entity.ID, ID, key);
        }
        public Vector2 GetVector2(string key)
        {
            Vector2 outVec2;
            InternalCalls.MeshRenderer_GetMaterialVector2(entity.ID, ID, key, out outVec2);
            return outVec2;
        }
        public Vector3 GetVector3(string key)
        {
            Vector3 outVec3;
            InternalCalls.MeshRenderer_GetMaterialVector3(entity.ID, ID, key, out outVec3);
            return outVec3;
        }
        public Vector4 GetVector4(string key)
        {
            Vector4 outVec4;
            InternalCalls.MeshRenderer_GetMaterialVector4(entity.ID, ID, key, out outVec4);
            return outVec4;
        }
        public Color GetColor(string key)
        {
            Vector4 outColor;
            InternalCalls.MeshRenderer_GetMaterialVector4(entity.ID, ID, key, out outColor);
            return new Color(outColor);
        }

        public void SetInt(string key, int value)
        {
            InternalCalls.MeshRenderer_SetMaterialInt(entity.ID, ID, key, value);
        }
        public void SetFloat(string key, float value)
        {
            InternalCalls.MeshRenderer_SetMaterialFloat(entity.ID, ID, key, value);
        }
        public void SetVector2(string key, Vector2 vec2)
        {
            InternalCalls.MeshRenderer_SetMaterialVector2(entity.ID, ID, key, ref vec2);
        }
        public void SetVector3(string key, Vector3 vec3)
        {
            InternalCalls.MeshRenderer_SetMaterialVector3(entity.ID, ID, key, ref vec3);
        }
        public void SetVector4(string key, Vector4 vec4)
        {
            InternalCalls.MeshRenderer_SetMaterialVector4(entity.ID, ID, key, ref vec4);
        }
        public void SetColor(string key, Color color)
        {
            Vector4 colorRGBA = color.rgba;
            InternalCalls.MeshRenderer_SetMaterialVector4(entity.ID, ID, key, ref colorRGBA);
        }


        public static Entity RaycastPickEntity(Vector3 origin, Vector3 direction, float maxDistance)
        {
            return new Entity(InternalCalls.MeshRenderer_RaycastEntityPick(ref origin, ref direction, maxDistance));
        }
    }
}