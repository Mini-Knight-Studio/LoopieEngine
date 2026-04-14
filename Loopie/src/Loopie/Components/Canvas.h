#pragma once

#include "Loopie/Components/UIElement.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Math/MathTypes.h"

namespace Loopie {

	enum class CanvasRenderMode
	{
		WorldSpace,
		ScreenSpaceOverlay,
	};

	class Canvas : public UIElement, public IObserver<TransformNotification>
	{
	public:
		DEFINE_TYPE(Canvas)

		Canvas() = default;
		~Canvas() override;

		void Init() override;

		void RenderGizmo() const override;

		void OnNotify(const TransformNotification& id) override;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

		CanvasRenderMode GetRenderMode() const { return m_renderMode; }
		void SetRenderMode(CanvasRenderMode mode) { m_renderMode = mode; }

	private:
		void RebuildWorldCornersIfNeeded() const;
		void SyncOverlayRectSizeIfNeeded();

	private:
		vec4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		bool m_drawGizmo = true;

		mutable bool m_cornersDirty = true;
		mutable vec3 m_worldCorners[4]{};

		CanvasRenderMode m_renderMode = CanvasRenderMode::ScreenSpaceOverlay;
		mutable ivec2 m_lastOverlayTargetPixels = ivec2(-1, -1);
	};
}