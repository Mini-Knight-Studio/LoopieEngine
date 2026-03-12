using System;

namespace Loopie
{
    public abstract class Component
    {
        public string ID { get; internal set; }
        public Entity entity { get; internal set; }
        public Transform transform => entity.transform;

        protected Component() { }
        protected Component(Entity entity, string ID) 
        { 
            this.entity = entity;
            this.ID = ID;
        }

        public static bool operator ==(Component a, Component b)
        {
            if (ReferenceEquals(a, b))
                return true;

            if (a is null || b is null)
                return false;

            return a.ID == b.ID && a.entity == b.entity;
        }

        public static bool operator !=(Component a, Component b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Component other)
                return this == other;

            return false;
        }

        public override int GetHashCode()
        {
            return ID != null ? ID.GetHashCode() : 0;
        }
    }
}