#pragma once

#include "Loopie/Components/Component.h"
#include "Loopie/Math/MathTypes.h"

namespace Loopie
{
	enum class CanvasScaleMode
	{
		ConstantPixelSize = 0,
		ScaleWithCanvasSize = 1,
	};

	class CanvasScaler : public Component
	{
	public:
		DEFINE_TYPE(CanvasScaler)

		CanvasScaler() = default;
		~CanvasScaler() override = default;

		void Init() override {};

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

		CanvasScaleMode GetScaleMode() const { return m_scaleMode; }
		void SetScaleMode(CanvasScaleMode mode) { m_scaleMode = mode; }

		vec2 GetReferenceResolution() const { return m_referenceResolution; }
		void SetReferenceResolution(const vec2& res) { m_referenceResolution = res; }

		vec2 ComputeScale(const vec2& targetPixels, const vec2& canvasUnits) const;
		vec2 ComputeOverlayCanvasSize(const vec2& targetPixels) const;

	private:
		CanvasScaleMode m_scaleMode = CanvasScaleMode::ScaleWithCanvasSize;
		vec2 m_referenceResolution = vec2(500.0f, 500.0f);
	};
}