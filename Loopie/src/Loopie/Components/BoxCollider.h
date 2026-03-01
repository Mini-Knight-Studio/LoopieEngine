#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Math/OBB.h"

namespace Loopie {

    class BoxCollider : public Component, public IObserver<TransformNotification> {
    public:
        DEFINE_TYPE(BoxCollider)

            BoxCollider();
        virtual ~BoxCollider();

        void Init() override;
        void RenderGizmo() override;

        void OnNotify(const TransformNotification& id) override;

        const OBB& GetWorldOBB() const;

        void SetLocalCenter(const vec3& center) { m_localCenter = center; m_obbDirty = true; }
        const vec3& GetLocalCenter() const { return m_localCenter; }

        void SetLocalExtents(const vec3& extents) { m_localExtents = extents; m_obbDirty = true; }
        const vec3& GetLocalExtents() const { return m_localExtents; }

        void SetDrawGizmo(bool value) { m_drawGizmo = value; }
        bool GetDrawGizmo() const { return m_drawGizmo; }

        bool Intersects(const BoxCollider* other) const;
        void OnUpdate() override;
        JsonNode Serialize(JsonNode& parent) const override;
        void Deserialize(const JsonNode& data) override;
        std::shared_ptr<Entity> CheckCollision();

    private:
        void RecalculateOBB() const;

    private:
        vec3 m_localCenter = vec3(0.0f);
        vec3 m_localExtents = vec3(0.5f);
        bool m_wasCollidingLastFrame = false;
        mutable OBB m_worldOBB;
        mutable bool m_obbDirty = true;
        bool m_drawGizmo = true;
    };
}