#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"

#include <memory>
#include <algorithm>

namespace Loopie
{
	class Texture;

	class SpriteAnimator : public Component
	{
	public:

		enum AnimationUpdateMode
		{
			DeltaTime,
			UnscaledDeltaTime
		};

		DEFINE_TYPE(SpriteAnimator)

		SpriteAnimator() = default;
		~SpriteAnimator() override = default;

		void Init() override;
		void OnUpdate() override;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

		std::shared_ptr<Texture> GetTexture() const { return m_texture; }
		void SetTexture(const std::shared_ptr<Texture>& texture);

		ivec2 GetGrid() const { return m_grid; }
		void SetGrid(const ivec2& grid);

		int GetStartFrame() const { return m_startFrame; }
		void SetStartFrame(int frame) { m_startFrame = std::max(0, frame); }

		int GetFrameCount() const { return m_frameCount; }
		void SetFrameCount(int count) { m_frameCount = std::max(1, count); }

		float GetFPS() const { return m_fps; }
		void SetFPS(float fps) { m_fps = std::max(0.0f, fps); }

		bool GetLoop() const { return m_loop; }
		void SetLoop(bool loop) { m_loop = loop; }

		bool GetPlayOnStart() const { return m_playOnStart; }
		void SetPlayOnStart(bool v) { m_playOnStart = v; }

		bool GetPlaying() const { return m_playing; }
		void SetPlaying(bool playing);

		int GetCurrentFrame() const;
		void SetCurrentFrame(int frameIndex);

		void Play();
		void Stop(bool resetTime = true);

		void SetAnimationMode(AnimationUpdateMode mode) { m_mode = mode; }
		AnimationUpdateMode GetAnimationMode() const { return m_mode; }

	private:
		vec4 ComputeUVRect(int frameIndex) const;
		void ApplyFrame(int frameIndex);

		float GetUpdateTime() const;

	private:
		std::shared_ptr<Texture> m_texture;
		ivec2 m_grid = ivec2(1, 1); // (columns, rows)
		int m_startFrame = 0;
		int m_frameCount = 1;
		float m_fps = 12.0f;
		bool m_loop = true;
		bool m_playOnStart = true;

		bool m_playing = true;
		double m_time = 0.0;
		int m_lastAppliedFrame = -1;

		AnimationUpdateMode m_mode = AnimationUpdateMode::DeltaTime;
	};
}
