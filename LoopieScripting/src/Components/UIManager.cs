namespace Loopie
{
    public class UIManager : Component
    {
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
