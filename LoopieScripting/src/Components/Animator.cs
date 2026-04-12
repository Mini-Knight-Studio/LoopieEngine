using System;

namespace Loopie
{
    public class Animator : Component
    {
        public void Stop()
        {
            InternalCalls.Animator_Stop(entity.ID, ID);
        }

        public void Play(string clipName, float transitionTime = 0.25f)
        {
            InternalCalls.Animator_PlayClip(entity.ID, ID, clipName, transitionTime);
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

        public float GetCurrentClipDuration()
        {
            return InternalCalls.Animator_GetCurrentClipDuration(entity.ID, ID);
        }

        public int GetNextClipIndex()
        {
            return InternalCalls.Animator_GetNextClipIndex(entity.ID, ID);
        }

        public string GetNextClipName()
        {
            return InternalCalls.Animator_GetNextClipName(entity.ID, ID);
        }

        public float GetNextClipDuration()
        {
            return InternalCalls.Animator_GetNextClipDuration(entity.ID, ID);
        }

        public string GetClipName(int index)
        {
            return InternalCalls.Animator_GetClipName(entity.ID, ID, index);
        }

        ////

        public int GetClipIndex(string clipName)
        {
            return InternalCalls.Animator_GetClipIndex(entity.ID, ID, clipName);
        }

        public float GetClipDuration(int index)
        {
            return InternalCalls.Animator_GetClipDurationByIndex(entity.ID, ID, index);
        }

        public float GetClipDuration(string clipName)
        {
            return InternalCalls.Animator_GetClipDurationByName(entity.ID, ID, clipName);
        }

        ////

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

        public bool InTransition
        { get { return IsInTransition(); } }

        private bool IsInTransition()
        {
            return InternalCalls.Animator_IsInTransition(entity.ID, ID);
        }

    }
}

