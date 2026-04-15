#include "SpriteAnimator.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Resources/Types/Texture.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"

#include "Loopie/Profiler/Profiler.h"

#include <algorithm>
#include <cmath>

namespace Loopie
{
	void SpriteAnimator::Init()
	{
		m_playing = m_playOnStart;
		m_time = 0.0;
		m_lastAppliedFrame = -1;

		// Ensure there's at least a texture on the linked Image.
		if (auto owner = GetOwner(); owner)
		{
			if (auto* image = owner->GetComponent<Image>(); image)
			{
				if (!m_texture)
					m_texture = image->GetTexture();
			}
		}
	}

	void SpriteAnimator::OnUpdate()
	{
		LP_FUNC();

		if (!GetIsActive() || !m_playing)
			return;

		if (!m_texture)
		{
			if (auto owner = GetOwner(); owner)
			{
				if (auto* image = owner->GetComponent<Image>(); image)
					m_texture = image->GetTexture();
			}
		}

		const int cols = std::max(1, m_grid.x);
		const int rows = std::max(1, m_grid.y);
		const int totalFrames = cols * rows;
		if (totalFrames <= 0)
			return;

		const int start = std::clamp(m_startFrame, 0, std::max(0, totalFrames - 1));
		const int count = std::clamp(m_frameCount, 1, std::max(1, totalFrames - start));
		const float fps = std::max(0.0f, m_fps);

		if (!m_texture)
			return;

		m_time += Time::GetDeltaTime();

		int localFrame = 0;
		if (fps > 0.0f)
			localFrame = (int)std::floor(m_time * (double)fps);

		int frameIndex = 0;
		if (m_loop)
		{
			frameIndex = start + (localFrame % count);
		}
		else
		{
			frameIndex = start + std::min(localFrame, count - 1);
			if (frameIndex >= start + count - 1)
				m_playing = false;
		}

		ApplyFrame(frameIndex);
	}

	void SpriteAnimator::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		m_texture = texture;
		m_lastAppliedFrame = -1;
		m_time = 0.0;
	}

	void SpriteAnimator::SetGrid(const ivec2& grid)
	{
		m_grid.x = std::max(1, grid.x);
		m_grid.y = std::max(1, grid.y);
		m_lastAppliedFrame = -1;
		m_time = 0.0;
	}

	void SpriteAnimator::SetPlaying(bool playing)
	{
		m_playing = playing;
		if (m_playing)
			m_lastAppliedFrame = -1;
	}

	void SpriteAnimator::Play()
	{
		m_playing = true;
		m_time = 0.0;
		m_lastAppliedFrame = -1;
	}

	void SpriteAnimator::Stop(bool resetTime)
	{
		m_playing = false;
		if (resetTime)
		{
			m_time = 0.0;

			const int cols = std::max(1, m_grid.x);
			const int rows = std::max(1, m_grid.y);
			const int totalFrames = cols * rows;
			if (totalFrames > 0)
			{
				const int start = std::clamp(m_startFrame, 0, std::max(0, totalFrames - 1));
				ApplyFrame(start);
			}
		}
	}

	int SpriteAnimator::GetCurrentFrame() const
	{
		const int cols = std::max(1, m_grid.x);
		const int rows = std::max(1, m_grid.y);
		const int totalFrames = cols * rows;
		if (totalFrames <= 0)
			return 0;

		if (m_lastAppliedFrame >= 0)
			return std::clamp(m_lastAppliedFrame, 0, totalFrames - 1);

		const int start = std::clamp(m_startFrame, 0, std::max(0, totalFrames - 1));
		const int count = std::clamp(m_frameCount, 1, std::max(1, totalFrames - start));
		const float fps = std::max(0.0f, m_fps);

		int localFrame = 0;
		if (fps > 0.0f)
			localFrame = (int)std::floor(m_time * (double)fps);

		if (m_loop)
			return start + (localFrame % count);
		return start + std::min(localFrame, count - 1);
	}

	vec4 SpriteAnimator::ComputeUVRect(int frameIndex) const
	{
		const int cols = std::max(1, m_grid.x);
		const int rows = std::max(1, m_grid.y);
		const int totalFrames = cols * rows;

		if (totalFrames <= 0)
			return vec4(0.0f, 0.0f, 1.0f, 1.0f);

		frameIndex = std::clamp(frameIndex, 0, totalFrames - 1);

		const int col = frameIndex % cols;
		const int row = frameIndex / cols;

		const float tileW = 1.0f / (float)cols;
		const float tileH = 1.0f / (float)rows;

		const float u0 = (float)col * tileW;
		const float u1 = (float)(col + 1) * tileW;

		// Row 0 is the TOP row (common spritesheet convention).
		const float v1 = 1.0f - (float)row * tileH;
		const float v0 = 1.0f - (float)(row + 1) * tileH;

		return vec4(u0, v0, u1, v1);
	}

	void SpriteAnimator::ApplyFrame(int frameIndex)
	{
		if (frameIndex == m_lastAppliedFrame)
			return;

		auto owner = GetOwner();
		if (!owner)
			return;

		auto* image = owner->GetComponent<Image>();
		if (!image)
			return;

		if (!m_texture)
			m_texture = image->GetTexture();

		if (!m_texture)
			return;

		if (image->GetTexture() != m_texture)
			image->SetTexture(m_texture);

		image->SetUVRect(ComputeUVRect(frameIndex));
		m_lastAppliedFrame = frameIndex;
	}

	JsonNode SpriteAnimator::Serialize(JsonNode& parent) const
	{
		JsonNode node = parent.CreateObjectField("sprite_animator");

		if (m_texture)
			node.CreateField<std::string>("texture_uuid", m_texture->GetUUID().Get());

		JsonNode grid = node.CreateObjectField("grid");
		grid.CreateField<int>("columns", m_grid.x);
		grid.CreateField<int>("rows", m_grid.y);

		node.CreateField<int>("start_frame", m_startFrame);
		node.CreateField<int>("frame_count", m_frameCount);
		node.CreateField<float>("fps", m_fps);
		node.CreateField<bool>("loop", m_loop);
		node.CreateField<bool>("play_on_start", m_playOnStart);
		node.CreateField<bool>("playing", m_playing);

		return node;
	}

	void SpriteAnimator::Deserialize(const JsonNode& data)
	{
		m_texture.reset();

		if (data.Contains("texture_uuid"))
		{
			UUID texUUID = data.GetValue<std::string>("texture_uuid").Result;
			Metadata* meta = AssetRegistry::GetMetadata(texUUID);
			if (meta)
			{
				auto tex = ResourceManager::GetTexture(*meta);
				if (tex && tex->Load())
					m_texture = tex;
			}
		}

		if (data.Contains("grid"))
		{
			JsonNode grid = data.Child("grid");
			if (grid.IsValid())
			{
				m_grid.x = std::max(1, grid.GetValue<int>("columns", 1).Result);
				m_grid.y = std::max(1, grid.GetValue<int>("rows", 1).Result);
			}
		}

		m_startFrame = std::max(0, data.GetValue<int>("start_frame", 0).Result);
		m_frameCount = std::max(1, data.GetValue<int>("frame_count", 1).Result);
		m_fps = std::max(0.0f, data.GetValue<float>("fps", 12.0f).Result);
		m_loop = data.GetValue<bool>("loop", true).Result;
		m_playOnStart = data.GetValue<bool>("play_on_start", true).Result;
		m_playing = data.GetValue<bool>("playing", m_playOnStart).Result;

		m_time = 0.0;
		m_lastAppliedFrame = -1;
	}
	void SpriteAnimator::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const SpriteAnimator& otherAnim = static_cast<const SpriteAnimator&>(other);

		m_texture = otherAnim.m_texture;
		m_grid = otherAnim.m_grid;
		m_startFrame = otherAnim.m_startFrame;
		m_frameCount = otherAnim.m_frameCount;
		m_fps = otherAnim.m_fps;
		m_loop = otherAnim.m_loop;
		m_playOnStart = otherAnim.m_playOnStart;
		m_playing = otherAnim.m_playing;
		m_time = 0.0;
		m_lastAppliedFrame = -1;
	}
}
