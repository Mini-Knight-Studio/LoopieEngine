namespace Loopie
{
    public class UIManager : Component
    {
        public bool BlockNavigation
        {
            get
            {
                return InternalCalls.UIManager_GetBlockNavigation(entity.ID, ID);
            }
            set
            {
                InternalCalls.UIManager_SetBlockNavigation(entity.ID, ID, value);
            }
        }

        public Entity SelectedElement
        {
            get
            {
                string id = InternalCalls.UIManager_GetSelectedEntity(entity.ID, ID);
                return new Entity(id);
            }
            set
            {
                string id = value != null ? value.ID : "";
                InternalCalls.UIManager_SetSelectedEntity(entity.ID, ID, id);
            }
        }

        public void ClearSelection()
        {
            InternalCalls.UIManager_SetSelectedEntity(entity.ID, ID, "");
        }
    }
}
