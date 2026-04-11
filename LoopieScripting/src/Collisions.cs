using System;
using System.ComponentModel;

namespace Loopie
{
    public struct Ray
    {
        public Vector3 origin;
        public Vector3 direction;

        public Ray(Vector3 origin, Vector3 direction)
        {
            this.origin = origin;
            this.direction = direction;
        }
    }

    public struct RaycastHit
    {
        internal string entityID;
        internal string componentID;
        public Vector3 point;
        public float distance;

        public Entity entity => new Entity(entityID);

        public BoxCollider collider
        {
            get
            {
                BoxCollider col = new BoxCollider();
                col.entity = entity;
                col.ID = componentID;
                return col;
            }
        }
    }

    public static class Collisions
    {
        public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit, int layerMask = -1, BoxCollider avoidCollider = null)
        {
            hit = new RaycastHit();
            if(avoidCollider == null)
                return InternalCalls.Collisions_Raycast(origin, direction, maxDistance, out hit, layerMask, "","");
            return InternalCalls.Collisions_Raycast(origin, direction, maxDistance, out hit, layerMask, avoidCollider.entity.ID, avoidCollider.ID);
        }

        public static int GetLayerBit(string layerName)
        {
            return InternalCalls.Collisions_GetLayerBit(layerName);
        }
    }
}