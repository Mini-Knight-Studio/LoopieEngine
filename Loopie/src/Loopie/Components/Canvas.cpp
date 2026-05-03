#include "Canvas.h"

#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/CanvasScaler.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Core/Log.h"

namespace Loopie {
	Canvas::~Canvas()
	{
		auto owner = GetOwner();
		if (!owner)
			return;

		if (RectTransform* rectTransform = owner->GetComponent<RectTransform>())
		{
			rectTransform->m_transformNotifier.RemoveObserver(this);
		}
	}

	void Canvas::Init()
	{
		auto owner = GetOwner();
		if (!owner)
			return;

		RectTransform* rectTransform = owner->GetComponent<RectTransform>();
		if (!rectTransform)
		{
			Log::Warn("Canvas requires RectTransform Gizmo will not update");
			return;
		}

		rectTransform->m_transformNotifier.AddObserver(this);
		SyncOverlayRectSizeIfNeeded();
		m_cornersDirty = true;
	}

	void Canvas::OnNotify(const TransformNotification& id)
	{
		if (id == TransformNotification::OnDirty || id == TransformNotification::OnChanged)
		{
			m_cornersDirty = true;
		}
	}

	void Canvas::RenderGizmo() const
	{
		if (!m_drawGizmo)
			return;

		RebuildWorldCornersIfNeeded();

		Gizmo::DrawLine(m_worldCorners[0], m_worldCorners[1], m_color);
		Gizmo::DrawLine(m_worldCorners[1], m_worldCorners[2], m_color);
		Gizmo::DrawLine(m_worldCorners[2], m_worldCorners[3], m_color);
		Gizmo::DrawLine(m_worldCorners[3], m_worldCorners[0], m_color);
	}

	void Canvas::RebuildWorldCornersIfNeeded() const
	{
		if (!m_cornersDirty)
			return;

		Transform* transform = GetTransform();
		if (!transform)
			return;

		const vec3 minL = transform->GetLocalBoundsMin();
		const vec3 maxL = transform->GetLocalBoundsMax();

		const vec3 bl = vec3(minL.x, minL.y, 0.0f);
		const vec3 br = vec3(maxL.x, minL.y, 0.0f);
		const vec3 tr = vec3(maxL.x, maxL.y, 0.0f);
		const vec3 tl = vec3(minL.x, maxL.y, 0.0f);

		const matrix4& m = transform->GetLocalToWorldMatrix();

		m_worldCorners[0] = vec3(m * vec4(bl, 1.0f));
		m_worldCorners[1] = vec3(m * vec4(br, 1.0f));
		m_worldCorners[2] = vec3(m * vec4(tr, 1.0f));
		m_worldCorners[3] = vec3(m * vec4(tl, 1.0f));

		m_cornersDirty = false;
	}

	void Canvas::SyncOverlayRectSizeIfNeeded()
	{
		if (m_renderMode != CanvasRenderMode::ScreenSpaceOverlay)
			return;

		auto owner = GetOwner();
		if (!owner)
			return;

		auto rectTransform = owner->GetComponent<RectTransform>();
		if (!rectTransform)
			return;

		const auto parent = owner->GetParent().lock();
		if (parent && parent->GetTransform() && parent->GetTransform()->HasSize())
			return;

		const ivec2 targetPixels = Application::GetInstance().GetWindow().GetSize();
		if (targetPixels.x <= 0 || targetPixels.y <= 0)
			return;

		if (targetPixels == m_lastOverlayTargetPixels)
			return;

		vec2 canvasSize = vec2((float)targetPixels.x, (float)targetPixels.y);
		if (auto* scaler = owner->GetComponent<CanvasScaler>())
			canvasSize = scaler->ComputeOverlayCanvasSize(canvasSize);

		rectTransform->SetWidth(canvasSize.x);
		rectTransform->SetHeight(canvasSize.y);

		m_lastOverlayTargetPixels = targetPixels;
		m_cornersDirty = true;
	}

	JsonNode Canvas::Serialize(JsonNode& parent) const
	{
		JsonNode canvasObj = parent.CreateObjectField("canvas");
		canvasObj.CreateField<bool>("draw_gizmo", m_drawGizmo);
		canvasObj.CreateField<int>("render_mode", static_cast<int>(m_renderMode));
		SerializeDrawOrder(canvasObj);
		SerializeNavigation(canvasObj);

		JsonNode colorObj = canvasObj.CreateObjectField("color");
		colorObj.CreateField("r", m_color.r);
		colorObj.CreateField("g", m_color.g);
		colorObj.CreateField("b", m_color.b);
		colorObj.CreateField("a", m_color.a);

		return canvasObj;
	}

	void Canvas::Deserialize(const JsonNode& data)
	{
		if (data.Contains("draw_gizmo"))
			m_drawGizmo = data.GetValue<bool>("draw_gizmo", true).Result;
		if (data.Contains("render_mode"))
			m_renderMode = static_cast<CanvasRenderMode>(data.GetValue<int>("render_mode", static_cast<int>(CanvasRenderMode::WorldSpace)).Result);
		DeserializeDrawOrder(data);
		DeserializeNavigation(data);

		JsonNode colorObj = data.Child("color");
		if (colorObj.IsValid() && colorObj.IsObject())
		{
			m_color.r = colorObj.GetValue<float>("r", m_color.r).Result;
			m_color.g = colorObj.GetValue<float>("g", m_color.g).Result;
			m_color.b = colorObj.GetValue<float>("b", m_color.b).Result;
			m_color.a = colorObj.GetValue<float>("a", m_color.a).Result;
		}

		m_cornersDirty = true;
	}

	void Canvas::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const Canvas& otherCanvas = static_cast<const Canvas&>(other);
		m_color = otherCanvas.m_color;
		m_drawGizmo = otherCanvas.m_drawGizmo;
		m_renderMode = otherCanvas.m_renderMode;
		CloneDrawOrder(otherCanvas);
		CloneNavigation(otherCanvas);
		m_cornersDirty = true;
	}
}