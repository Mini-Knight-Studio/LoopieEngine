using System;

namespace Loopie
{
    public sealed class Text : Component
    {
        public Vector4 Color
        {
            get => GetColor();
            set => SetColor(value);
        }

        public Vector4 GetColor()
        {
            Vector4 color;
            InternalCalls.Text_GetColor(entity.ID, ID, out color);
            return color;
        }

        public void SetColor(Vector4 color)
        {
            InternalCalls.Text_SetColor(entity.ID, ID, ref color);
        }
    }
}