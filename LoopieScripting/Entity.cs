using System;
using System.Runtime.CompilerServices;

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
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            if (InternalCalls.Entity_HasComponent(ID, componentType))
            {
                if (!InternalCalls.Entity_GetComponent(ID, componentType, out string componentID))
                    return null;
                T component = new T();
                component.entity = this;
                component.ID = componentID;
                return component;
            }

            string typeName = typeof(T).FullName;
            object scriptInstance = InternalCalls.Entity_GetScriptInstance(ID, typeName);

            return scriptInstance as T;
        }

        public static Entity FindEntityByName(string name)
        {
            string entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == "")
                return null;

            return new Entity(entityID);
        }

        public static Entity FindEntityByID(string entityID)
        {
            entityID = InternalCalls.Entity_FindEntityByID(entityID);
            if (entityID == "")
                return null;

            return new Entity(entityID);
        }

        public static Entity Instantiate(string name)
        {
            string instanceId = InternalCalls.Entity_Create(name,null);
            if (instanceId == "")
                return null;

            return new Entity(instanceId);
        }

        public T AddComponent<T>() where T : Component, new()
        {
            string componentType = typeof(T).FullName;
            if(InternalCalls.Entity_AddComponent(ID, componentType, out string componentID))
            {
                T component = new T();
                component.entity = this;
                component.ID = componentID;
                return component;
            }
            return null;
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
            if (instanceId == "")
                return null;

            return new Entity(instanceId);
        }

        public Entity Clone(Entity entity, bool cloneChilds = false)
        {
            string instanceId = InternalCalls.Entity_Clone(entity.ID, cloneChilds); ;
            if (instanceId == "")
                return null;

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
    }
}