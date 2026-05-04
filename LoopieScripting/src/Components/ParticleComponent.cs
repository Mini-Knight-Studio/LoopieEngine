using System;

namespace Loopie
{
    public class ParticleComponent : Component
    {
        // Playback
        public void Play()
        {
            InternalCalls.ParticleSystem_Play(entity.ID, ID);
        }

        public void Stop()
        {
            InternalCalls.ParticleSystem_Stop(entity.ID, ID);
        }

        public bool IsPlaying => InternalCalls.ParticleSystem_IsPlaying(entity.ID, ID);

        // Emitters
        public int GetEmitterIndex(string emitterName)
        {
            return InternalCalls.ParticleSystem_GetEmitterIndex(entity.ID, ID, emitterName);
        }

        public void SetEmitterState(int index, bool state)
        {
            InternalCalls.ParticleSystem_SetEmitterState(entity.ID, ID, index, state);
        }

        public bool GetEmitterState(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterState(entity.ID, ID, index);
        }

        public void SetEmitterName(int index, string name)
        {
            InternalCalls.ParticleSystem_SetEmitterName(entity.ID, ID, index, name);
        }

        public string GetEmitterName(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterName(entity.ID, ID, index);
        }

        // Spawn / Limits
        public int GetSpawnRate(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterSpawnRate(entity.ID, ID, index);
        }

        public void SetSpawnRate(int index, int rate)
        {
            InternalCalls.ParticleSystem_SetEmitterSpawnRate(entity.ID, ID, index, rate);
        }

        public int GetMaxParticles(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterMaxParticles(entity.ID, ID, index);
        }

        public void SetMaxParticles(int index, int max)
        {
            InternalCalls.ParticleSystem_SetEmitterMaxParticles(entity.ID, ID, index, max);
        }

        // Transform
        public Vector3 GetPosition(int index)
        {
            Vector3 pos = Vector3.Zero;
            InternalCalls.ParticleSystem_GetEmitterPosition(entity.ID, ID, index, out pos);
            return pos;
        }

        public void SetPosition(int index, Vector3 position)
        {
            InternalCalls.ParticleSystem_SetEmitterPosition(entity.ID, ID, index, ref position);
        }

        public Vector3 GetPositionOffset(int index)
        {
            Vector3 pos = Vector3.Zero;
            InternalCalls.ParticleSystem_GetEmitterPositionOffset(entity.ID, ID, index, out pos);
            return pos;
        }

        public void SetPositionOffset(int index, Vector3 offset)
        {
            InternalCalls.ParticleSystem_SetEmitterPositionOffset(entity.ID, ID, index, ref offset);
        }

        public Vector3 GetRotation(int index)
        {
            Vector3 rot = Vector3.Zero;
            InternalCalls.ParticleSystem_GetEmitterRotation(entity.ID, ID, index, out rot);
            return rot;
        }

        public void SetRotation(int index, Vector3 rotation)
        {
            InternalCalls.ParticleSystem_SetEmitterRotation(entity.ID, ID, index, ref rotation);
        }

        // Behaviour
        public bool GetFollowParent(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterFollowParent(entity.ID, ID, index);
        }

        public void SetFollowParent(int index, bool value)
        {
            InternalCalls.ParticleSystem_SetEmitterFollowParent(entity.ID, ID, index, value);
        }

        public bool GetLocalVelocity(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterLocalVelocity(entity.ID, ID, index);
        }

        public void SetLocalVelocity(int index, bool value)
        {
            InternalCalls.ParticleSystem_SetEmitterLocalVelocity(entity.ID, ID, index, value);
        }

        // Emission Properties
        public Vector3 GetVelocity(int index)
        {
            Vector3 v = Vector3.Zero;
            InternalCalls.ParticleSystem_GetEmitterPropVelocity(entity.ID, ID, index, out v);
            return v;
        }

        public void SetVelocity(int index, Vector3 velocity)
        {
            InternalCalls.ParticleSystem_SetEmitterPropVelocity(entity.ID, ID, index, ref velocity);
        }

        public Vector3 GetVelocityVariation(int index)
        {
            Vector3 v = Vector3.Zero;
            InternalCalls.ParticleSystem_GetEmitterPropVelocityVariation(entity.ID, ID, index, out v);
            return v;
        }

        public void SetVelocityVariation(int index, Vector3 variation)
        {
            InternalCalls.ParticleSystem_SetEmitterPropVelocityVariation(entity.ID, ID, index, ref variation);
        }

        public Vector3 GetPositionVariation(int index)
        {
            Vector3 v = Vector3.Zero;
            InternalCalls.ParticleSystem_GetEmitterPropPositionVariation(entity.ID, ID, index, out v);
            return v;
        }

        public void SetPositionVariation(int index, Vector3 variation)
        {
            InternalCalls.ParticleSystem_SetEmitterPropPositionVariation(entity.ID, ID, index, ref variation);
        }

        // Color
        public Vector4 GetColorBegin(int index)
        {
            InternalCalls.ParticleSystem_GetEmitterPropColorBegin(entity.ID, ID, index, out Vector4 c);
            return c;
        }

        public void SetColorBegin(int index, Vector4 color)
        {
            InternalCalls.ParticleSystem_SetEmitterPropColorBegin(entity.ID, ID, index, ref color);
        }

        public Vector4 GetColorEnd(int index)
        {
            InternalCalls.ParticleSystem_GetEmitterPropColorEnd(entity.ID, ID, index, out Vector4 c);
            return c;
        }

        public void SetColorEnd(int index, Vector4 color)
        {
            InternalCalls.ParticleSystem_SetEmitterPropColorEnd(entity.ID, ID, index, ref color);
        }

        // Size
        public float GetSizeBegin(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterPropSizeBegin(entity.ID, ID, index);
        }

        public void SetSizeBegin(int index, float size)
        {
            InternalCalls.ParticleSystem_SetEmitterPropSizeBegin(entity.ID, ID, index, size);
        }

        public float GetSizeEnd(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterPropSizeEnd(entity.ID, ID, index);
        }

        public void SetSizeEnd(int index, float size)
        {
            InternalCalls.ParticleSystem_SetEmitterPropSizeEnd(entity.ID, ID, index, size);
        }

        public float GetSizeVariation(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterSizeVariation(entity.ID, ID, index);
        }

        public void SetSizeVariation(int index, float variation)
        {
            InternalCalls.ParticleSystem_SetEmitterSizeVariation(entity.ID, ID, index, variation);
        }

        // Lifetime
        public float GetLifetime(int index)
        {
            return InternalCalls.ParticleSystem_GetEmitterPropLifetime(entity.ID, ID, index);
        }

        public void SetLifetime(int index, float lifetime)
        {
            InternalCalls.ParticleSystem_SetEmitterPropLifetime(entity.ID, ID, index, lifetime);
        }
    }
}