#include "Loopie/Components/Component.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Math/MathTypes.h"

namespace Loopie {
	class Canvas : public Component, public IObserver<TransformNotification>
	{
	public:
		DEFINE_TYPE(Canvas)

		Canvas() = default;
		~Canvas() override;

		void Init() override;

		void RenderGizmo() override;

		void OnNotify(const TransformNotification& id) override;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		void RebuildWorldCornersIfNeeded() const;

	private:
		vec4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		bool m_drawGizmo = true;

		mutable bool m_cornersDirty = true;
		mutable vec3 m_worldCorners[4]{};
	};
}