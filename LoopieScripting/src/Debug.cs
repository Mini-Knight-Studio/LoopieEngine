namespace Loopie
{
    public static class Debug
    {
        
        public static void Log(string value)
        {
            InternalCalls.NativeLog(value,2);
        }

        public static void Log(int value)
        {
            InternalCalls.NativeLog_Int(value,2);
        }

        public static void Log(float value)
        {
            InternalCalls.NativeLog_Float(value,2);
        }

        public static void Log(Vector2 value)
        {
            InternalCalls.NativeLog_Vector2(value,2);
        }

        public static void Log(Vector3 value)
        {
            InternalCalls.NativeLog_Vector3(value,2);
        }

        public static void Log(Vector4 value)
        {
            InternalCalls.NativeLog_Vector4(value,2);
        }




        public static void LogWarning(string value)
        {
            InternalCalls.NativeLog(value,3);
        }

        public static void LogWarning(int value)
        {
            InternalCalls.NativeLog_Int(value,3);
        }

        public static void LogWarning(float value)
        {
            InternalCalls.NativeLog_Float(value,3);
        }

        public static void LogWarning(Vector2 value)
        {
            InternalCalls.NativeLog_Vector2(value,3);
        }

        public static void LogWarning(Vector3 value)
        {
            InternalCalls.NativeLog_Vector3(value,3);
        }

        public static void LogWarning(Vector4 value)
        {
            InternalCalls.NativeLog_Vector4(value,3);
        }





        public static void LogError(string value)
        {
            InternalCalls.NativeLog(value,4);
        }

        public static void LogError(int value)
        {
            InternalCalls.NativeLog_Int(value,4);
        }

        public static void LogError(float value)
        {
            InternalCalls.NativeLog_Float(value,4);
        }

        public static void LogError(Vector2 value)
        {
            InternalCalls.NativeLog_Vector2(value,4);
        }

        public static void LogError(Vector3 value)
        {
            InternalCalls.NativeLog_Vector3(value,4);
        }

        public static void LogError(Vector4 value)
        {
            InternalCalls.NativeLog_Vector4(value,4);
        }
    }
}