using System;

namespace Loopie
{
    public class Animator : Component
    {
        public void Stop()
        {
            InternalCalls.Animator_Stop(entity.ID, ID);
        }

        public void Play(string clipName)
        {
            InternalCalls.Animator_PlayClip(entity.ID, ID, clipName);
        }

        public void Play()
        {
            InternalCalls.Animator_Play(entity.ID, ID);
        }

        public void Pause()
        {
            InternalCalls.Animator_Pause(entity.ID, ID);
        }

        public void Resume()
        {
            InternalCalls.Animator_Resume(entity.ID, ID);
        }

        public int GetCurrentClipIndex()
        {
            return InternalCalls.Animator_GetCurrentClipIndex(entity.ID, ID);
        }

        public string GetCurrentClipName()
        {
            return InternalCalls.Animator_GetCurrentClipName(entity.ID, ID);
        }

        public string GetClipName(int index)
        {
            return InternalCalls.Animator_GetClipName(entity.ID, ID, index);
        }

        public float GetPlaybackSpeed()
        {
            return InternalCalls.Animator_GetPlaybackSpeed(entity.ID, ID);
        }

        public void SetPlaybackSpeed(float speed)
        {
            InternalCalls.Animator_SetPlaybackSpeed(entity.ID, ID, speed);
        }

        public bool Looping
        { get { return IsLooping(); } set { SetLooping(value); } }

        private void SetLooping(bool isLooping)
        {
            InternalCalls.Animator_SetLooping(entity.ID, ID, isLooping);
        }

        private bool IsLooping()
        {
            return InternalCalls.Animator_IsLooping(entity.ID, ID);
        }

        public bool Playing
        { get { return IsPlaying(); } }

        private bool IsPlaying()
        {
            return InternalCalls.Animator_IsPlaying(entity.ID, ID);
        }

        public float GetCurrentTime()
        {
            return InternalCalls.Animator_GetCurrentTime(entity.ID, ID);
        }

    }
}

