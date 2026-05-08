#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Events/EventTypes.h"

#include "Loopie/Resources/Types/Mesh.h"
#include "Loopie/Resources/Types/Material.h"

#include <memory>

namespace Loopie {

	class Animator;

	struct Triangle
	{
		vec3 v0, v1, v2;
	};

	class MeshRenderer : public Component, public IObserver<TransformNotification> {
	public:
		DEFINE_TYPE(MeshRenderer)

		MeshRenderer();
		~MeshRenderer();
		void Init() override; //// From Component
		void OnNotify(const TransformNotification& id) override;

		void RenderGizmo() const override;
		
		std::shared_ptr<Mesh> GetMesh() { return m_mesh; }
		void SetMesh(std::shared_ptr<Mesh> mesh);

		std::shared_ptr<Material> GetMaterial();
		std::shared_ptr<Material> GetInstancedMaterial();
		void SetMaterial(std::shared_ptr <Material> material);
		
		void CreateInstanceMaterial();

		void SetBoundingBoxesDirty() { m_boundingBoxesDirty = true; }
		const AABB& GetWorldAABB() const;
		const OBB& GetWorldOBB() const;

		///TEST
		void SetDrawNormalsPerFace(bool value) { m_drawNormalsPerFace = value; }
		bool GetDrawNormalsPerFace() { return m_drawNormalsPerFace; }
		void SetDrawNormalsPerTriangle(bool value) { m_drawNormalsPerTriangle = value; }
		bool GetDrawNormalsPerTriangle() { return m_drawNormalsPerTriangle; }
		void SetDrawAABB(bool value) { m_drawAABB = value; }
		bool GetDrawAABB() { return m_drawAABB; }
		void SetDrawOBB(bool value) { m_drawOBB = value; }
		bool GetDrawOBB() { return m_drawOBB; }
		void SetCastsShadows(bool value) { m_castsShadows = value; }
		bool GetCastsShadows() const { return m_castsShadows; }
		///TEST

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

		bool GetTriangle(int triangleIndex, Triangle& triangle) const;

		void SetLinkedAnimator(Animator* animator) { m_linkedAnimator = animator; }
		Animator* GetLinkedAnimator() { return m_linkedAnimator; }

	private:
		void RecalculateBoundingBoxes() const;

		///TEST
		vec3 GetVertexVec3Data(const MeshData& data, unsigned int vertexIndex, unsigned int offset) const;
		void RenderNormalsPerFace(float length, const vec4& color) const;
		void RenderNormalsPerTriangle(float length, const vec4& color) const;
		bool m_drawNormalsPerFace = false;
		bool m_drawNormalsPerTriangle = false;
		bool m_drawAABB = false;
		bool m_drawOBB = false;
		///TEST

	private:
		std::shared_ptr<Material> m_material;
		std::shared_ptr<Material> m_instanceMaterial;
		std::shared_ptr<Mesh> m_mesh;

		mutable AABB m_worldAABB = AABB();
		mutable OBB m_worldOBB = OBB();
		mutable bool m_boundingBoxesDirty = true;
		bool m_castsShadows = true;

		Animator* m_linkedAnimator = nullptr;
	};
}