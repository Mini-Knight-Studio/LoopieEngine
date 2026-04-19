using System;

namespace Loopie
{
    public class Image : Component
    {
        public int SortingLayer
        {
            get => InternalCalls.UIElement_GetSortingLayer(entity.ID, ID);
            set => InternalCalls.UIElement_SetSortingLayer(entity.ID, ID, value);
        }

        public int OrderInLayer
        {
            get => InternalCalls.UIElement_GetOrderInLayer(entity.ID, ID);
            set => InternalCalls.UIElement_SetOrderInLayer(entity.ID, ID, value);
        }

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