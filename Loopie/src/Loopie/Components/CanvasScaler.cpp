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

	vec2 CanvasScaler::ComputeScale(const vec2& targetPixels, const vec2& canvasUnits) const
	{
		if (m_scaleMode == CanvasScaleMode::ConstantPixelSize)
			return vec2(1.0f);

		if (m_scaleMode == CanvasScaleMode::ScaleWithCanvasSize)
		{
			float widthScale = targetPixels.x / m_referenceResolution.x;
			float heightScale = targetPixels.y / m_referenceResolution.y;

			float scale = glm::mix(widthScale, heightScale, m_matchWidthOrHeight);

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
			float widthScale = targetPixels.x / m_referenceResolution.x;
			float heightScale = targetPixels.y / m_referenceResolution.y;

			float scale = glm::mix(widthScale, heightScale, m_matchWidthOrHeight);

			return targetPixels / scale;
		}

		return targetPixels;
	}
}
