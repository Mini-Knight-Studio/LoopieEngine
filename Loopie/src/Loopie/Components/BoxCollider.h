#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Events/IObserver.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Math/OBB.h"
#include "Loopie/Math/AABB.h"
#include <vector>

namespace Loopie {

    class BoxCollider : public Component, public IObserver<TransformNotification> {
		friend class CollisionProcessor;
    public:
        DEFINE_TYPE(BoxCollider)

        BoxCollider();
        virtual ~BoxCollider();

        void Init() override;
        void RenderGizmo() override;

        const OBB& GetWorldOBB() const;
        const AABB& GetWorldAABB() const;

        void SetLocalCenter(const vec3& center) { m_localCenter = center; m_obbDirty = true; }
        const vec3& GetLocalCenter() const { return m_localCenter; }

        void SetLocalExtents(const vec3& extents) { m_localExtents = extents; m_obbDirty = true; }
        const vec3& GetLocalExtents() const { return m_localExtents; }

        void SetDrawGizmo(bool value) { m_drawGizmo = value; }
        bool GetDrawGizmo() const { return m_drawGizmo; }

        void SetIsTrigger(bool trigger) { m_isTrigger = trigger; }
        bool IsTrigger() const { return m_isTrigger; }

        void SetIsStatic(bool isStatic) { m_isStatic = isStatic; }
        bool IsStatic() const { return m_isStatic; }

        bool Intersects(const BoxCollider* other) const;
        const unsigned int GetLayerIndex() const { return m_layerIndex; }
        const unsigned int GetLayerBit() const;
        void SetLayer(const std::string& name);
        void SetLayer(int index);
        bool IsColliding() const { return m_colliding; }
        bool CollidedThisFrame() const { return m_collided; }
        bool StoppedColliding() const { return m_stopColliding; }

        const std::vector<BoxCollider*>& GetCollidingWith() const { return m_collidingWith; }

        JsonNode Serialize(JsonNode& parent) const override;
        void Deserialize(const JsonNode& data) override;

    private:
        void RecalculateOBB() const;
        void OnNotify(const TransformNotification& id) override;
    private:
        vec3 m_localCenter = vec3(0.0f);
        vec3 m_localExtents = vec3(0.5f);
        unsigned int  m_layerIndex = 0;

        std::vector<BoxCollider*> m_collidingWith;

        bool m_wasCollidingLastFrame = false;
        bool m_colliding = false;
        bool m_collided = false;
        bool m_stopColliding = false;

        mutable AABB m_cachedAABB;
        mutable OBB m_worldOBB;

        bool m_isTrigger = false;
        bool m_isStatic = false;

        mutable bool m_obbDirty = true;
        bool m_drawGizmo = true;
    };
}