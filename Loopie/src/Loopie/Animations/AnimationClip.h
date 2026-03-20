#pragma once
#include "Loopie/Math/MathUtils.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Loopie {

	struct Vec3Key {
		vec3 Value;
		float Time;

		static vec3 Interpolate(const std::vector<Vec3Key>& keyframes, float time)
		{
			if (keyframes.empty()) 
				return vec3(0.0f);
			if (keyframes.size() == 1) 
				return keyframes[0].Value;

			for (size_t i = 0; i < keyframes.size() - 1; ++i)
			{
				if (time < keyframes[i + 1].Time)
				{
					const auto& p0 = keyframes[i];
					const auto& p1 = keyframes[i + 1];

					float dt = p1.Time - p0.Time;
					if (dt < Math::EPSILON) 
						return p0.Value;

					float factor = glm::clamp((time - p0.Time) / dt, 0.0f, 1.0f);
					return glm::mix(p0.Value, p1.Value, factor);
				}
			}
			return keyframes.back().Value;
		}
	};

	struct QuaternionKey
	{
		quaternion Value;
		float Time;

		static quaternion Interpolate(const std::vector<QuaternionKey>& keyframes, float time)
		{
			if (keyframes.empty()) 
				return quaternion(1.0f, 0.0f, 0.0f, 0.0f);
			if (keyframes.size() == 1) 
				return keyframes[0].Value;

			for (size_t i = 0; i < keyframes.size() - 1; ++i)
			{
				if (time < keyframes[i + 1].Time)
				{
					const auto& r0 = keyframes[i];
					const auto& r1 = keyframes[i + 1];

					float dt = r1.Time - r0.Time;
					if (dt < Math::EPSILON)
						return r0.Value;

					float factor = glm::clamp((time - r0.Time) / dt, 0.0f, 1.0f);
					return glm::slerp(r0.Value, r1.Value, factor);
				}
			}
			return keyframes.back().Value;
		}
	};

	struct KeyFrame
	{
		std::vector<Vec3Key> Positions;
		std::vector<QuaternionKey> Rotations;
		std::vector<Vec3Key> Scales;
	};

	struct AnimationClip
	{
		std::string Name = "";
		float Duration = 0.0f;
		std::unordered_map<std::string, KeyFrame> KeyFrames;


	};
}