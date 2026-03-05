#pragma once
#include "Loopie/Math/MathTypes.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Loopie {

	struct Vec3Key {
		vec3 Value;
		float Time;

		static vec3 Interpolate(const std::vector<Vec3Key>& keyframes, float time)
		{
			if (keyframes.size() == 1)
				return keyframes[0].Value;

			for (size_t i = 0; i < keyframes.size() - 1; ++i)
			{
				if (time < keyframes[i + 1].Time)
				{
					const auto& p0 = keyframes[i];
					const auto& p1 = keyframes[i + 1];

					float factor = (time - p0.Time) / (p1.Time - p0.Time);
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
			if (keyframes.size() == 1)
				return keyframes[0].Value;

			for (size_t i = 0; i < keyframes.size() - 1; ++i)
			{
				if (time < keyframes[i + 1].Time)
				{
					const auto& r0 = keyframes[i];
					const auto& r1 = keyframes[i + 1];

					float factor = (time - r0.Time) / (r1.Time - r0.Time);
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