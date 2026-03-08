namespace Loopie
{
    public static class Time
    {
        public static float deltaTime
        {
            get { return GetDeltaTime(); }
        }
        private static float GetDeltaTime()
        {
            return InternalCalls.Time_GetDeltaTime();
        }

        public static float fixedDeltaTime
        {
            get { return GetFixedDeltaTime(); }
        }
        private static float GetFixedDeltaTime()
        {
            return InternalCalls.Time_GetFixedDeltaTime();
        }

        public static float timeScale
        {
            get { return GetTimeScale(); }
            set { SetTimeScale(value); }
        }
        private static float GetTimeScale()
        {
            return InternalCalls.Time_GetTimeScale();
        }
        private static void SetTimeScale(float scale)
        {
            InternalCalls.Time_SetTimeScale(scale);
        }
    }
}