using System;

namespace Loopie
{
    public sealed class Text : Component
    {
        public string Value
        {
            get => GetText();
            set => SetText(value);
        }
        public Vector4 Color
        {
            get => GetColor();
            set => SetColor(value);
        }

        private string GetText()
        {
            return InternalCalls.Text_GetText(entity.ID, ID); ;
        }

        public void SetText(string text)
        {
            InternalCalls.Text_SetText(entity.ID, ID, text);
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