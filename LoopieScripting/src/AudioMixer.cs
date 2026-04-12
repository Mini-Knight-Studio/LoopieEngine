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

    }
}