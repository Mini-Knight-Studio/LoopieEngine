#include "CanvasScaler.h"

namespace Loopie
{
	JsonNode CanvasScaler::Serialize(JsonNode& parent) const
	{
		JsonNode obj = parent.CreateObjectField("canvas_scaler");
		obj.CreateField<int>("scale_mode", static_cast<int>(m_scaleMode));
		obj.CreateField<float>("ref_w", m_referenceResolution.x);
		obj.CreateField<float>("ref_h", m_referenceResolution.y);
		obj.CreateField<float>("match_width_or_height", m_matchWidthOrHeight);
		return obj;
	}

	void CanvasScaler::Deserialize(const JsonNode& data)
	{
		if (data.Contains("scale_mode"))
			m_scaleMode = static_cast<CanvasScaleMode>(data.GetValue<int>("scale_mode", (int)CanvasScaleMode::ScaleWithCanvasSize).Result);

		m_referenceResolution.x = data.GetValue<float>("ref_w", m_referenceResolution.x).Result;
		m_referenceResolution.y = data.GetValue<float>("ref_h", m_referenceResolution.y).Result;
		m_matchWidthOrHeight = data.GetValue<float>("match_width_or_height", m_matchWidthOrHeight).Result;
	}

	void CanvasScaler::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const CanvasScaler& otherScaler = static_cast<const CanvasScaler&>(other);
		m_scaleMode = otherScaler.m_scaleMode;
		m_referenceResolution = otherScaler.m_referenceResolution;
		m_matchWidthOrHeight = otherScaler.m_matchWidthOrHeight;
	}

	vec2 CanvasScaler::ComputeScale(const vec2& targetPixels, const vec2& canvasUnits) const
	{
		if (m_scaleMode == CanvasScaleMode::ConstantPixelSize)
			return vec2(1.0f);

		if (m_scaleMode == CanvasScaleMode::ScaleWithCanvasSize)
		{
			(void)canvasUnits;

			const float refW = glm::max(m_referenceResolution.x, 1.0f);
			const float refH = glm::max(m_referenceResolution.y, 1.0f);

			const float widthScale = targetPixels.x / refW;
			const float heightScale = targetPixels.y / refH;

			const float match = glm::clamp(m_matchWidthOrHeight, 0.0f, 1.0f);
			const float logWidth = glm::log2(glm::max(widthScale, 0.0001f));
			const float logHeight = glm::log2(glm::max(heightScale, 0.0001f));
			const float logWeighted = glm::mix(logWidth, logHeight, match);
			const float scale = glm::exp2(logWeighted);

			return vec2(scale, scale);
		}

		return vec2(1.0f);
	}

	vec2 CanvasScaler::ComputeOverlayCanvasSize(const vec2& targetPixels) const
	{
		if (m_scaleMode == CanvasScaleMode::ConstantPixelSize)
			return targetPixels;

		if (m_scaleMode == CanvasScaleMode::ScaleWithCanvasSize)
		{
			const float refW = glm::max(m_referenceResolution.x, 1.0f);
			const float refH = glm::max(m_referenceResolution.y, 1.0f);

			const float widthScale = targetPixels.x / refW;
			const float heightScale = targetPixels.y / refH;

			const float match = glm::clamp(m_matchWidthOrHeight, 0.0f, 1.0f);
			const float logWidth = glm::log2(glm::max(widthScale, 0.0001f));
			const float logHeight = glm::log2(glm::max(heightScale, 0.0001f));
			const float logWeighted = glm::mix(logWidth, logHeight, match);
			const float scale = glm::exp2(logWeighted);

			if (scale <= 0.0f)
				return targetPixels;

			return targetPixels / scale;
		}

		return targetPixels;
	}
}
