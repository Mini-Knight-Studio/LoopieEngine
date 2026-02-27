#include "Canvas.h"

#include "Loopie/Components/RectTransform.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Core/Log.h"


Loopie::Canvas::~Canvas()
{
	auto owner = GetOwner();
	if (!owner)
		return;

	if (RectTransform* rectTransform = owner->GetComponent<RectTransform>())
	{
		rectTransform->m_transformNotifier.RemoveObserver(this);
	}
}

void Loopie::Canvas::Init()
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
	m_cornersDirty = true;
}

void Loopie::Canvas::OnNotify(const TransformNotification& id)
{
	if (id == TransformNotification::OnDirty || id == TransformNotification::OnChanged)
	{
		m_cornersDirty = true;
	}
}

void Loopie::Canvas::RenderGizmo()
{
	if (!m_drawGizmo)
		return;

	RebuildWorldCornersIfNeeded();

	Gizmo::DrawLine(m_worldCorners[0], m_worldCorners[1], m_color);
	Gizmo::DrawLine(m_worldCorners[1], m_worldCorners[2], m_color);
	Gizmo::DrawLine(m_worldCorners[2], m_worldCorners[3], m_color);
	Gizmo::DrawLine(m_worldCorners[3], m_worldCorners[0], m_color);
}

void Loopie::Canvas::RebuildWorldCornersIfNeeded() const
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

Loopie::JsonNode Loopie::Canvas::Serialize(JsonNode& parent) const
{
	JsonNode canvasObj = parent.CreateObjectField("canvas");
	canvasObj.CreateField<bool>("draw_gizmo", m_drawGizmo);
	canvasObj.CreateField<int>("render_mode", static_cast<int>(m_renderMode));

	JsonNode colorObj = canvasObj.CreateObjectField("color");
	colorObj.CreateField("r", m_color.r);
	colorObj.CreateField("g", m_color.g);
	colorObj.CreateField("b", m_color.b);
	colorObj.CreateField("a", m_color.a);

	return canvasObj;
}

void Loopie::Canvas::Deserialize(const JsonNode& data)
{
	if (data.Contains("draw_gizmo"))
		m_drawGizmo = data.GetValue<bool>("draw_gizmo", true).Result;
	if (data.Contains("render_mode"))
		m_renderMode = static_cast<CanvasRenderMode>(data.GetValue<int>("render_mode", static_cast<int>(CanvasRenderMode::WorldSpace)).Result);

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
