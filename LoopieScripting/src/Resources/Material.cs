using System;

namespace Loopie
{
    public sealed class Material : Resource
    {
        internal Material(string resourceID, int index) : base(resourceID, index) { }

        public int GetInt(string key)
        {
            return InternalCalls.Material_GetInt(ID, key);
        }
        public float GetFloat(string key)
        {
            return InternalCalls.Material_GetFloat(ID, key);
        }
        public Vector2 GetVector2(string key)
        {
            Vector2 outVec2;
            InternalCalls.Material_GetVector2(ID, key, out outVec2);
            return outVec2;
        }
        public Vector3 GetVector3(string key)
        {
            Vector3 outVec3;
            InternalCalls.Material_GetVector3(ID, key, out outVec3);
            return outVec3;
        }
        public Vector4 GetVector4(string key)
        {
            Vector4 outVec4;
            InternalCalls.Material_GetVector4(ID, key, out outVec4);
            return outVec4;
        }
        public Color GetColor(string key)
        {
            Vector4 outColor;
            InternalCalls.Material_GetVector4(ID, key, out outColor);
            return new Color(outColor);
        }

        public void SetInt(string key, int value)
        {
            InternalCalls.Material_SetInt(ID, key, value);
        }
        public void SetFloat(string key, float value)
        {
            InternalCalls.Material_SetFloat(ID, key, value);
        }
        public void SetVector2(string key, Vector2 vec2)
        {
            InternalCalls.Material_SetVector2(ID, key, ref vec2);
        }
        public void SetVector3(string key, Vector3 vec3)
        {
            InternalCalls.Material_SetVector3(ID, key, ref vec3);
        }
        public void SetVector4(string key, Vector4 vec4)
        {
            InternalCalls.Material_SetVector4(ID, key, ref vec4);
        }
        public void SetColor(string key, Color color)
        {
            Vector4 colorRGBA = color.rgba;
            InternalCalls.Material_SetVector4(ID, key, ref colorRGBA);
        }
    }
}