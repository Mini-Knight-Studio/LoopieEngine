namespace Loopie
{
    public static class SceneManager
    {
        public static bool LoadSceneByID(string sceneID)
        {
            return InternalCalls.Scene_LoadByID(sceneID);
        }
    }
}