using System;

namespace Loopie
{
    public class Button : Component
    {
        public enum VisualTransitionMode
        {
            ColorTint = 0,
            TextureSwap = 1
        }

        public bool Interactable
        {
            get => InternalCalls.Button_IsInteractable(entity.ID, ID);
            set => InternalCalls.Button_SetInteractable(entity.ID, ID, value);
        }

        public bool Hovered => InternalCalls.Button_IsHovered(entity.ID, ID);

        public bool Pressed => InternalCalls.Button_IsPressed(entity.ID, ID);

        public void Click()
        {
            InternalCalls.Button_TriggerClick(entity.ID, ID);
        }

        public VisualTransitionMode TransitionMode
        {
            get => (VisualTransitionMode)InternalCalls.Button_GetTransitionMode(entity.ID, ID);
            set => InternalCalls.Button_SetTransitionMode(entity.ID, ID, (int)value);
        }

        public Vector4 NormalColor
        {
            get { Vector4 c; InternalCalls.Button_GetNormalColor(entity.ID, ID, out c); return c; }
            set { InternalCalls.Button_SetNormalColor(entity.ID, ID, ref value); }
        }

        public Vector4 HoveredColor
        {
            get { Vector4 c; InternalCalls.Button_GetHoveredColor(entity.ID, ID, out c); return c; }
            set { InternalCalls.Button_SetHoveredColor(entity.ID, ID, ref value); }
        }

        public Vector4 PressedColor
        {
            get { Vector4 c; InternalCalls.Button_GetPressedColor(entity.ID, ID, out c); return c; }
            set { InternalCalls.Button_SetPressedColor(entity.ID, ID, ref value); }
        }

        public Vector4 DisabledColor
        {
            get { Vector4 c; InternalCalls.Button_GetDisabledColor(entity.ID, ID, out c); return c; }
            set { InternalCalls.Button_SetDisabledColor(entity.ID, ID, ref value); }
        }

        public string NormalTextureUUID
        {
            get => InternalCalls.Button_GetNormalTextureUUID(entity.ID, ID);
            set => InternalCalls.Button_SetNormalTextureUUID(entity.ID, ID, value);
        }

        public string HoveredTextureUUID
        {
            get => InternalCalls.Button_GetHoveredTextureUUID(entity.ID, ID);
            set => InternalCalls.Button_SetHoveredTextureUUID(entity.ID, ID, value);
        }

        public string PressedTextureUUID
        {
            get => InternalCalls.Button_GetPressedTextureUUID(entity.ID, ID);
            set => InternalCalls.Button_SetPressedTextureUUID(entity.ID, ID, value);
        }

        public string DisabledTextureUUID
        {
            get => InternalCalls.Button_GetDisabledTextureUUID(entity.ID, ID);
            set => InternalCalls.Button_SetDisabledTextureUUID(entity.ID, ID, value);
        }
    }
}
