using System;

namespace Loopie
{
    public abstract class Resource
    {
        public string ID { get; internal set; }
        public int Index { get; internal set; }

        internal Resource(string id, int index)
        {
            ID = id;
            Index = index;
        }

        public T GetAs<T>() where T : Resource
        {
            return this as T;
        }

        public static bool operator ==(Resource a, Resource b)
        {
            bool aIsNullLike = a is null || a.IsNullResource();
            bool bIsNullLike = b is null || b.IsNullResource();

            if (aIsNullLike && bIsNullLike)
                return true;

            if (aIsNullLike || bIsNullLike)
                return false;

            return a.ID == b.ID;
        }

        public static bool operator !=(Resource a, Resource b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Resource other)
                return this == other;

            return false;
        }

        public override int GetHashCode()
        {
            return ID != null ? ID.GetHashCode() : 0;
        }

        private bool IsNullResource()
        {
            return string.IsNullOrEmpty(ID);
        }
    }
}