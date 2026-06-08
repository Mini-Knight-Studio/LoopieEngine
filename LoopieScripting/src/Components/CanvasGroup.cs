using System;
using System.ComponentModel;

namespace Loopie
{
    public sealed class CanvasGroup : Component
    {
        public float Alpha
        {
            get => InternalCalls.CanvasGroup_GetAlpha(entity.ID, ID);
            set => InternalCalls.CanvasGroup_SetAlpha(entity.ID, ID, value);
        }
    }
}