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
    }
}