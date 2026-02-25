using System;

namespace Loopie
{
    public abstract class Component
    {
        public Entity entity { get; internal set; }
        public Transform transform => entity.transform;
        protected Component() { }
        protected Component(Entity entity) 
        { 
            this.entity = entity;
        }
    }
}