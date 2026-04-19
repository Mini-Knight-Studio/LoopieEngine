using System;

namespace Loopie
{
    public class SpriteAnimator : Component
    {
        public string TextureUUID
        {
            get => InternalCalls.SpriteAnimator_GetTextureUUID(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetTextureUUID(entity.ID, ID, value);
        }

        public void GetGrid(out int columns, out int rows)
        {
            InternalCalls.SpriteAnimator_GetGrid(entity.ID, ID, out columns, out rows);
        }

        public void SetGrid(int columns, int rows)
        {
            InternalCalls.SpriteAnimator_SetGrid(entity.ID, ID, columns, rows);
        }

        public int StartFrame
        {
            get => InternalCalls.SpriteAnimator_GetStartFrame(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetStartFrame(entity.ID, ID, value);
        }

        public int FrameCount
        {
            get => InternalCalls.SpriteAnimator_GetFrameCount(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetFrameCount(entity.ID, ID, value);
        }

        public float FPS
        {
            get => InternalCalls.SpriteAnimator_GetFPS(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetFPS(entity.ID, ID, value);
        }

        public bool Loop
        {
            get => InternalCalls.SpriteAnimator_GetLoop(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetLoop(entity.ID, ID, value);
        }

        public bool PlayOnStart
        {
            get => InternalCalls.SpriteAnimator_GetPlayOnStart(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetPlayOnStart(entity.ID, ID, value);
        }

        public bool Playing
        {
            get => InternalCalls.SpriteAnimator_GetPlaying(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetPlaying(entity.ID, ID, value);
        }

        public void Play()
        {
            InternalCalls.SpriteAnimator_Play(entity.ID, ID);
        }

        public void Stop(bool resetTime = true)
        {
            InternalCalls.SpriteAnimator_Stop(entity.ID, ID, resetTime);
        }

        public int CurrentFrame
        {
            get => InternalCalls.SpriteAnimator_GetCurrentFrame(entity.ID, ID);
            set => InternalCalls.SpriteAnimator_SetCurrentFrame(entity.ID, ID, value);
        }    
    }
}
