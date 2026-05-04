namespace Loopie
{
    public static class Gizmo
    {
        public static void DrawLine(Vector3 start, Vector3 end, Color color)
        {
            Vector4 colorValue = color.rgba;
            InternalCalls.Gizmo_DrawLine(ref start, ref end, ref colorValue);
        }

        public static void DrawCircle(Vector3 center, float radius, Vector3 normal , float steps, Color color)
        {
            Vector4 colorValue = color.rgba;
            InternalCalls.Gizmo_DrawCircle(ref center, radius, ref normal, steps, ref colorValue);
        }

        public static void DrawSphere(Vector3 center, float radius, float steps, Color color)
        {
            Vector4 colorValue = color.rgba;
            InternalCalls.Gizmo_DrawSphere(ref center, radius, steps, ref colorValue);
        }
    }
}