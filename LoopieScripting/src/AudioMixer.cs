namespace Loopie
{
    public static class AudioMixer
    {

        public static void SetVolume(string channel, float volume)
        {
            InternalCalls.AudioMixer_SetBusVolume(channel, volume);
        }

        public static float GetVolume(string channel)
        {
            return InternalCalls.AudioMixer_GetBusVolume(channel);
        }

        public static float LinearToEngineVolume(float t)
        {
            t = Mathf.Clamp01(t);

            if (t <= 0.001f)
                return 0f;

            return Mathf.Pow(t, 2.2f);
        }

        public static float EngineToLinearVolume(float value)
        {
            value = Mathf.Clamp01(value);
            return Mathf.Sqrt(value);
        }

    }
}