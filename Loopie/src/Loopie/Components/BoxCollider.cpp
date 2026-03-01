#include "BoxCollider.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
namespace Loopie {

    BoxCollider::BoxCollider() {}

    BoxCollider::~BoxCollider() {
        if (GetOwner() && GetOwner()->HasComponent<Transform>()) {
            Transform* t = GetTransform();
            if (t) {
                t->m_transformNotifier.RemoveObserver(this);
            }
        }
    }
    void BoxCollider::Init() {
        GetTransform()->m_transformNotifier.AddObserver(this);
        RecalculateOBB();
    }

    void BoxCollider::OnNotify(const TransformNotification& id) {
        if (id == TransformNotification::OnChanged || id == TransformNotification::OnDirty) {
            m_obbDirty = true;
        }
    }

    void BoxCollider::RenderGizmo() {
        if (m_drawGizmo) {
            vec4 color = m_wasCollidingLastFrame ? vec4(1.0f, 0.0f, 0.0f, 1.0f) : vec4(0.0f, 1.0f, 0.0f, 1.0f);

            Gizmo::DrawCube(GetWorldOBB().GetCorners(), color);
        }
    }

    const OBB& BoxCollider::GetWorldOBB() const {
        RecalculateOBB();
        return m_worldOBB;
    }

    void BoxCollider::RecalculateOBB() const {
        if (!m_obbDirty) return;

        m_worldOBB.Center = m_localCenter;
        m_worldOBB.Extents = m_localExtents;
        m_worldOBB.Axes[0] = vec3(1, 0, 0);
        m_worldOBB.Axes[1] = vec3(0, 1, 0);
        m_worldOBB.Axes[2] = vec3(0, 0, 1);

        m_worldOBB.ApplyTransform(GetTransform()->GetLocalToWorldMatrix());
        m_obbDirty = false;
    }

    bool BoxCollider::Intersects(const BoxCollider* other) const {
        if (!other) return false;
        return GetWorldOBB().Intersects(other->GetWorldOBB());
    }
    void BoxCollider::OnUpdate()
    {
        if (!GetIsActive()) return;

        auto& scene = Application::GetInstance().GetScene();
        const auto& entities = scene.GetAllEntities();

        bool isCurrentlyColliding = false;

        for (const auto& [uuid, entity] : entities)
        {
            if (entity == GetOwner() || !entity->GetIsActive()) continue;

            if (entity->HasComponent<BoxCollider>())
            {
                auto* other = entity->GetComponent<BoxCollider>();

                if (other && other->GetIsActive() && this->Intersects(other))
                {
                    isCurrentlyColliding = true;

                    if (!m_wasCollidingLastFrame)
                    {
                        Log::Info("DEBUG COLLISION: [{0}] touching [{1}]",
                            GetOwner()->GetName(),
                            entity->GetName());
                    }
                    break;
                }
            }
        }

        m_wasCollidingLastFrame = isCurrentlyColliding;
    }
    JsonNode BoxCollider::Serialize(JsonNode& parent) const {
        JsonNode node = parent.CreateObjectField("boxcollider");

        JsonNode centerNode = node.CreateObjectField("center");
        centerNode.CreateField<double>("x", static_cast<double>(m_localCenter.x));
        centerNode.CreateField<double>("y", static_cast<double>(m_localCenter.y));
        centerNode.CreateField<double>("z", static_cast<double>(m_localCenter.z));

        JsonNode extentsNode = node.CreateObjectField("extents");
        extentsNode.CreateField<double>("x", static_cast<double>(m_localExtents.x));
        extentsNode.CreateField<double>("y", static_cast<double>(m_localExtents.y));
        extentsNode.CreateField<double>("z", static_cast<double>(m_localExtents.z));

        node.CreateField<bool>("draw_gizmo", m_drawGizmo);

        return node;
    }

    void BoxCollider::Deserialize(const JsonNode& data) {
        if (data.Contains("center")) {
            JsonNode centerNode = data.Child("center");
            m_localCenter.x = static_cast<float>(centerNode.GetValue<double>("x", 0.0).Result);
            m_localCenter.y = static_cast<float>(centerNode.GetValue<double>("y", 0.0).Result);
            m_localCenter.z = static_cast<float>(centerNode.GetValue<double>("z", 0.0).Result);
        }

        if (data.Contains("extents")) {
            JsonNode extentsNode = data.Child("extents");
            m_localExtents.x = static_cast<float>(extentsNode.GetValue<double>("x", 0.5).Result);
            m_localExtents.y = static_cast<float>(extentsNode.GetValue<double>("y", 0.5).Result);
            m_localExtents.z = static_cast<float>(extentsNode.GetValue<double>("z", 0.5).Result);
        }

        if (data.Contains("draw_gizmo")) {
            m_drawGizmo = data.GetValue<bool>("draw_gizmo", true).Result;
        }

        m_obbDirty = true;
    }

}