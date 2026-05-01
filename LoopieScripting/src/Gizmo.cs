namespace Loopie
{
    public static class Gizmo
    {
        public static void DrawLine(Vector3 start, Vector3 end, Color color)
        {
            InternalCalls.Gizmo_DrawLine(start, end, color.rgba);
        }

        public static void DrawCircle(Vector3 center, float radius, Vector3 normal , float steps, Color color)
        {
            InternalCalls.Gizmo_DrawCircle(center, radius, normal, steps, color.rgba);
        }

        public static void DrawSphere(Vector3 center, float radius, float steps, Color color)
        {
            InternalCalls.Gizmo_DrawSphere(center, radius, steps, color.rgba);
        }
    }
}