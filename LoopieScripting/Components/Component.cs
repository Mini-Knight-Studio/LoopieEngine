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
    }
}