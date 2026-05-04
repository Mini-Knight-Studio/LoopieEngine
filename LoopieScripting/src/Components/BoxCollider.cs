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
            InternalCalls.BoxCollider_SetLocalCenter(entity.ID, ID, ref center);
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
            InternalCalls.BoxCollider_SetLocalExtents(entity.ID, ID, ref extents);
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

        public void SetIncludeMask(int includeMask)
        {
            InternalCalls.BoxCollider_SetIncludeMask(entity.ID, ID, includeMask);
        }

        public int GetIncludeMask()
        {
            return InternalCalls.BoxCollider_GetIncludeMask(entity.ID, ID);
        }

        public void AddIncludeMask(int includeMask)
        {
            InternalCalls.BoxCollider_AddIncludeMask(entity.ID, ID, includeMask);
        }

        public void RemoveIncludeMask(int includeMask)
        {
            InternalCalls.BoxCollider_RemoveIncludeMask(entity.ID, ID, includeMask);
        }

        public void SetExcludeMask(int excludeMask)
        {
            InternalCalls.BoxCollider_SetExcludeMask(entity.ID, ID, excludeMask);
        }

        public int GetExcludeMask()
        {
            return InternalCalls.BoxCollider_GetExcludeMask(entity.ID, ID);
        }

        public void AddExcludeMask(int excludeMask)
        {
            InternalCalls.BoxCollider_AddExcludeMask(entity.ID, ID, excludeMask);
        }

        public void RemoveExcludeMask(int excludeMask)
        {
            InternalCalls.BoxCollider_RemoveExcludeMask(entity.ID, ID, excludeMask);
        }
    }
}