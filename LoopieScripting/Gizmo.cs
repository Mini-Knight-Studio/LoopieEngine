namespace Loopie
{
    public static class Gizmo
    {
        public static void DrawLine(Vector3 start, Vector3 end, Color color)
        {
            InternalCalls.Gizmo_DrawLine(start, end, color.rgba);
        }
    }
}