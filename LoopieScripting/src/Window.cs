namespace Loopie
{
    public static class Window
    {
        public static int TargetFramerate
        {
            get { return GetTargetFramerate(); }
            set { SetTargetFramerate(value); }
        }
        private static int GetTargetFramerate()
        {
            return InternalCalls.Window_GetTargetFramerate();
        }
        private static void SetTargetFramerate(int targetFramerate)
        {
            InternalCalls.Window_SetTargetFramerate(targetFramerate);
        }

        public static Vector2 Size
        {
            get { return GetSize(); }
            set { SetSize(value); }
        }
        private static Vector2 GetSize()
        {
            Vector2 size = Vector2.Zero;
            InternalCalls.Window_GetSize(out size);
            return size;
        }
        private static void SetSize(Vector2 size)
        {
            InternalCalls.Window_SetSize(size);
        }

        public static void SetResizable(bool resizable)
        {
            InternalCalls.Window_SetResizable(resizable);
        }

        public static bool Fullscreen
        {
            get { return GetFullscreen(); }
            set { SetFullscreen(value); }
        }

        private static void SetFullscreen(bool fullscreen)
        {
            InternalCalls.Window_SetFullscreen(fullscreen);
        }

        private static bool GetFullscreen()
        {
            return InternalCalls.Window_GetFullscreen();
        }

        public static bool VSync
        {
            get { return GetVSync(); }
            set { SetVSync(value); }
        }

        private static void SetVSync(bool enabled)
        {
            InternalCalls.Window_SetVSync(enabled);
        }

        private static bool GetVSync()
        {
            return InternalCalls.Window_GetVSync();
        }
    }
}