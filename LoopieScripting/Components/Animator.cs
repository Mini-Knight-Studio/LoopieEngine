using System;

namespace Loopie
{
    public class Animator : Component
    {
        public void Stop()
        {
            InternalCalls.Animator_Stop(entity.ID);
        }

        public void Play(string clipName)
        {
            InternalCalls.Animator_Play(entity.ID, clipName);
        }

        public int GetCurrentClipIndex()
        {
            return InternalCalls.Animator_GetCurrentClipIndex(entity.ID);
        }

        public string GetCurrentClipName()
        {
            return InternalCalls.Animator_GetCurrentClipName(entity.ID);
        }

        public string GetClipName(int index)
        {
            return InternalCalls.Animator_GetClipName(entity.ID, index);
        }

        public float GetPlaybackSpeed()
        {
            return InternalCalls.Animator_GetPlaybackSpeed(entity.ID);
        }

        public void SetPlaybackSpeed(float speed)
        {
            InternalCalls.Animator_SetPlaybackSpeed(entity.ID, speed);
        }

        public bool Looping
        { get { return IsLooping(); } set { SetLooping(value); } }

        private void SetLooping(bool isLooping)
        {
            InternalCalls.Animator_SetLooping(entity.ID, isLooping);
        }

        private bool IsLooping()
        {
            return InternalCalls.Animator_IsLooping(entity.ID);
        }

        public bool Playing
        { get { return IsPlaying(); } }

        private bool IsPlaying()
        {
            return InternalCalls.Animator_IsPlaying(entity.ID);
        }

        public float GetCurrentTime()
        {
            return InternalCalls.Animator_GetCurrentTime(entity.ID);
        }

    }
}

