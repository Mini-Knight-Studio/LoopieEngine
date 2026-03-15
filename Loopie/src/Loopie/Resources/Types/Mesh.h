#pragma once
#include "Loopie/Resources/Resource.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Math/AABB.h"
#include "Loopie/Math/OBB.h"

#include "Loopie/Render/IndexBuffer.h"
#include "Loopie/Render/VertexBuffer.h"
#include "Loopie/Render/VertexArray.h"

#include "Loopie/Animations/AnimationClip.h"

#include <vector>
#include <memory>

namespace Loopie {

	constexpr int MAX_BONE_INFLUENCE = 4;


	struct Bone
	{
		int ID = -1;
		int ParentID = -1;
		std::string Name;
		matrix4 OffsetMatrix = matrix4(1);
		matrix4 LocalBindTransform = matrix4(1);

	};

	struct VertexBoneData
	{
		int IDs[MAX_BONE_INFLUENCE] = { 0,0,0,0 };
		float Weights[MAX_BONE_INFLUENCE] = { 0,0,0,0 };

		int Index = 0;

		void AddBoneData(int id, float weight)
		{
			if (Index == MAX_BONE_INFLUENCE)
				return;

			IDs[Index] = id;
			Weights[Index] = weight;
			Index++;
		}
	};

	struct MeshData {
		std::string Name;

		AABB BoundingBox;
		vec3 Position = vec3(0); /// Still NotWorking
		quaternion Rotation = quaternion(1,0,0,0); /// Still NotWorking
		vec3 Scale = vec3(1); /// Still NotWorking

		unsigned int VerticesAmount = 0;
		unsigned int VertexElements = 0;
		unsigned int IndicesAmount = 0;

		bool HasPosition = true;
		bool HasTexCoord = false;
		bool HasNormal = false;
		bool HasTangent = false;
		bool HasColor = false;
		bool HasBones = false;

		std::vector<float> Vertices;
		std::vector<unsigned int> Indices;

		std::vector<VertexBoneData> Bones;
		std::vector<Bone> Skeleton;

		matrix4 GlobalInverseTransform = matrix4(1);
		std::vector<AnimationClip> AnimationClips;
		const AnimationClip* GetAnimationClip(const std::string& name) const {
			for (const auto& clip : AnimationClips) {
				if (clip.Name == name) {
					return &clip;
				}
			}
			return nullptr;
		}

	};
	
	class Mesh : public Resource{
		friend class MeshRenderer;
		friend class MeshImporter;
	public :
		DEFINE_TYPE(Mesh)

		Mesh(const UUID& id, unsigned int index);
		~Mesh() = default;

		bool Load() override;

		const MeshData& GetData() { return m_data; }
		unsigned int GetMeshIndex() { return m_meshIndex; }
		const std::shared_ptr<VertexArray> GetVAO() { return m_vao; }
	private:
		MeshData m_data;

		std::shared_ptr<VertexArray> m_vao;
		std::shared_ptr<VertexBuffer> m_vbo;
		std::shared_ptr<VertexBuffer> m_boneIDVBO;
		std::shared_ptr<IndexBuffer> m_ebo;

		unsigned int m_meshIndex = 0;

	};
}