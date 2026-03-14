using System;

namespace Loopie
{
    public class BoxCollider : Component
    {
        public Vector3 LocalCenter {get { return GetLocalCenter(); } set { SetLocalCenter(value); }}
        public string Layer
        {
            get
            {
                return InternalCalls.BoxCollider_GetLayer(entity.ID, ID);
            }
            set
            {
                InternalCalls.BoxCollider_SetLayer(entity.ID, ID, value);
            }
        }
       

        private Vector3 GetLocalCenter()
        {
            Vector3 center = Vector3.Zero;
            InternalCalls.BoxCollider_GetLocalCenter(entity.ID, ID, out center);
            return center;
        }

        private void SetLocalCenter(Vector3 center)
        {
            InternalCalls.BoxCollider_SetLocalCenter(entity.ID, ID, center);
        }

        public Vector3 LocalExtents { get { return GetLocalExtents(); } set { SetLocalExtents(value); }}

        private Vector3 GetLocalExtents()
        {
            Vector3 extents = Vector3.Zero;
            InternalCalls.BoxCollider_GetLocalExtents(entity.ID, ID, out extents);
            return extents;
        }

        private void SetLocalExtents(Vector3 extents)
        {
            InternalCalls.BoxCollider_SetLocalExtents(entity.ID, ID, extents);
        }

        public bool IsColliding { get { return GetIsColliding(); }}

        private bool GetIsColliding()
        {
            return InternalCalls.BoxCollider_IsColliding(entity.ID, ID);
        }

        public bool HasCollided { get { return GetIfHasCollided(); } }

        private bool GetIfHasCollided()
        {
            return InternalCalls.BoxCollider_HasCollided(entity.ID, ID);
        }

        public bool HasEndedCollision { get { return GetIfHasEndedCollision(); } }

        private bool GetIfHasEndedCollision()
        {
            return InternalCalls.BoxCollider_HasEndedCollision(entity.ID, ID);
        }

        public bool Trigger { get { return IsTrigger(); } set { SetTrigger(value); } }

        private bool IsTrigger()
        {
            return InternalCalls.BoxCollider_IsTrigger(entity.ID, ID);
        }

        private void SetTrigger(bool isTrigger)
        {
            InternalCalls.BoxCollider_SetTrigger(entity.ID, ID, isTrigger);
        }

        public bool Static { get { return IsStatic(); } set { SetStatic(value); } }

        private bool IsStatic()
        {
            return InternalCalls.BoxCollider_IsStatic(entity.ID, ID);
        }

        private void SetStatic(bool isStatic)
        {
            InternalCalls.BoxCollider_SetStatic(entity.ID, ID, isStatic);
        }
    }
}