using System;

namespace Loopie
{
    public class AudioSource : Component
    {
       

        public void Play()
        {
            InternalCalls.AudioSource_Play(entity.ID);
        }

        public void Stop()
        {
            InternalCalls.AudioSource_Stop(entity.ID);
        }

        public bool Looping
        { get { return IsLooping(); } set { SetLoop(value); } }

        private bool IsLooping()
        {
            return InternalCalls.AudioSource_IsLooping(entity.ID);
        }

        private void SetLoop(bool value)
        {
            InternalCalls.AudioSource_SetLoop(entity.ID, value);
        }

        public float Pitch
        { get { return GetPitch(); } set { SetPitch(value); } }

        private float GetPitch()
        {
            return InternalCalls.AudioSource_GetPitch(entity.ID);
        }

        private void SetPitch(float value)
        {
            InternalCalls.AudioSource_SetPitch(entity.ID, value);
        }


        public float Volume
        { get { return GetVolume(); } set { SetVolume(value); } }

        private float GetVolume()
        {
            return InternalCalls.AudioSource_GetVolume(entity.ID);
        }

        private void SetVolume(float value)
        {
            InternalCalls.AudioSource_SetVolume(entity.ID, value);
        }


        public float Pan
        { get { return GetPan(); } set { SetPan(value); } }

        private float GetPan()
        {
            return InternalCalls.AudioSource_GetPan(entity.ID);
        }

        private void SetPan(float value)
        {
            InternalCalls.AudioSource_SetPan(entity.ID, value);
        }

        public void Set3DMinMaxDistance(float min, float max)
        {
            InternalCalls.AudioSource_SetSet3DMinMaxDistance(entity.ID, min, max);
        }

        public void Get3DMinMaxDistance(out float min, out float max)
        {
            InternalCalls.AudioSource_GetSet3DMinMaxDistance(entity.ID, out min, out max);
        }
    }

}