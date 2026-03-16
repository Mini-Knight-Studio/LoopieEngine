using System;

namespace Loopie
{
    public sealed class Image : Component
    {
        public Vector4 Tint
        {
            get => GetTint();
            set => SetTint(value);
        }

        public Vector4 GetTint()
        {
            Vector4 tint;
            InternalCalls.Image_GetTint(entity.ID, ID, out tint);
            return tint;
        }

        public void SetTint(Vector4 tint)
        {
            InternalCalls.Image_SetTint(entity.ID, ID, ref tint);
        }
    }
}