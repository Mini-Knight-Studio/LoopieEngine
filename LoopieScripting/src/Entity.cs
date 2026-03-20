using System;
using System.Collections;
using System.Collections.Generic;

namespace Loopie
{
    public class Entity
    {
        internal Entity(string id)
        {
            ID = id;
            transform = new Transform();
            transform.entity = this;
        }
        public readonly string ID;
        public Transform transform { get; }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType) || InternalCalls.Entity_GetScriptInstance(ID, componentType.FullName)!=null;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);

            T component = new T();
            component.entity = this;
            component.ID = "";

            if (InternalCalls.Entity_HasComponent(ID, componentType))
            {
                if (!InternalCalls.Entity_GetComponent(ID, componentType, out string componentID))
                    return component;
                component.ID = componentID;
                return component;
            }

            string typeName = typeof(T).FullName;
            object scriptInstance = InternalCalls.Entity_GetScriptInstance(ID, typeName);
            if (scriptInstance != null)
            {
                return scriptInstance as T;
            }
            return component;
        }

        public static Entity FindEntityByName(string name)
        {
            string entityID = InternalCalls.Entity_FindEntityByName(name);
            return new Entity(entityID);
        }

        public static Entity FindEntityByID(string entityID)
        {
            entityID = InternalCalls.Entity_FindEntityByID(entityID);
            return new Entity(entityID);
        }

        public static Entity Instantiate(string name)
        {
            string instanceId = InternalCalls.Entity_Create(name, null);
            return new Entity(instanceId);
        }

        public T AddComponent<T>() where T : Component, new()
        {
            string componentType = typeof(T).FullName;

            T component = new T();
            component.entity = this;
            component.ID = "";
            if (InternalCalls.Entity_AddComponent(ID, componentType, out string componentID))
            {
                component.ID = componentID;
                return component;
            }
            return component;
        }

        public static void Destroy(Entity entity)
        {
            InternalCalls.Entity_Destroy(entity.ID);
        }

        public void Destroy()
        {
            InternalCalls.Entity_Destroy(ID);
        }

        public Entity Clone(bool cloneChilds = false)
        {
            string instanceId = InternalCalls.Entity_Clone(ID, cloneChilds); ;
            return new Entity(instanceId);
        }

        public Entity Clone(Entity entity, bool cloneChilds = false)
        {
            string instanceId = InternalCalls.Entity_Clone(entity.ID, cloneChilds); ;
            return new Entity(instanceId);
        }

        public bool Active
        {
            get { return IsActive(); }
        }

        public bool ActiveInHierarchy
        {
            get { return IsActiveInHierarchy(); }
        }

        public void SetActive(bool active)
        {
            InternalCalls.Entity_SetActive(ID, active);
        }

        private bool IsActive()
        {
            return InternalCalls.Entity_IsActive(ID);
        }

        private bool IsActiveInHierarchy()
        {
            return InternalCalls.Entity_IsActiveInHierarchy(ID);
        }


        public Entity Parent
        {
            get { return GetParent(); }
            set { SetParent(value); }
        }

        private void SetParent(Entity parent)
        {
            string parentID = parent != null ? parent.ID : null;
            InternalCalls.Entity_SetParent(ID, parentID);
        }

        private Entity GetParent()
        {
            string parentID = InternalCalls.Entity_GetParent(ID);
            return new Entity(parentID);
        }

        public string Name
        {
            get { return GetName(); }
            set { SetName(value); }
        }

        private void SetName(string name)
        {
            InternalCalls.Entity_SetName(ID, name);
        }

        private string GetName()
        {
            return InternalCalls.Entity_GetName(ID);
        }

        public int ChildCount
        {
            get { return GetChildCount(); }
        }

        private int GetChildCount()
        {
            return InternalCalls.Entity_GetChildCount(ID);
        }

        public IEnumerable<Entity> Children
        {
            get
            {
                for (int i = 0; i < ChildCount; i++)
                    yield return GetChild(i);
            }
        }

        public List<Entity> GetChildren()
        {
            List<Entity> list = new List<Entity>();

            for (int i = 0; i < ChildCount; i++)
                list.Add(GetChild(i));

            return list;
        }

        public Entity GetChild(int index)
        {
            string childID = InternalCalls.Entity_GetChild(ID, index);
            return new Entity(childID);
        }

        public static bool operator ==(Entity a, Entity b)
        {
            if (ReferenceEquals(a, b))
                return true;

            if (a is null || b is null)
                return false;

            if (a.IsNullEntity() || b.IsNullEntity())
                return false;

            return a.ID == b.ID;
        }

        public static bool operator !=(Entity a, Entity b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Entity other)
                return this == other;

            return false;
        }

        public override int GetHashCode()
        {
            return ID != null ? ID.GetHashCode() : 0;
        }

        private bool IsNullEntity()
        {       
            return string.IsNullOrEmpty(ID);
        }
    }
}