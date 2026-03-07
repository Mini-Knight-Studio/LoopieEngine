using System.ComponentModel;

namespace Loopie
{
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
        public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit)
        {
            hit = new RaycastHit();
            return InternalCalls.Collisions_Raycast(origin, direction, maxDistance, out hit);
        }
    }
}