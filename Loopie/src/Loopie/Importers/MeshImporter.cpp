#include "MeshImporter.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Resources/Types/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <iostream>
#include <filesystem> // Used for checking the extension


namespace Loopie {
	void MeshImporter::ImportModel(const std::string& filepath, Metadata& metadata) {
		if (metadata.HasCache && !metadata.IsOutdated)
			return;


		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes |
			aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality);

		if (!scene || !scene->mRootNode) {
			Log::Error("Assimp Error: {0}", importer.GetErrorString());
			return;
		}

		metadata.CachesPath.clear();
		metadata.HasCache = true;
		metadata.Type = ResourceType::MESH;
		ProcessNode(scene->mRootNode, scene, metadata.CachesPath);

		MetadataRegistry::SaveMetadata(filepath, metadata);

		Log::Trace("Mesh Imported -> {0}", filepath);
	}

	void MeshImporter::LoadModel(const std::string& path, Mesh& mesh)
	{
		Project project = Application::GetInstance().m_activeProject;
		std::filesystem::path filepath = project.GetChachePath() / path;

		if (!std::filesystem::exists(filepath))
			return;

		std::ifstream file(filepath, std::ios::binary);
		if (!file)
		{
			Log::Warn("Error opening .mesh file -> {0}", filepath.string());
			return;
		}

		MeshData data;

		unsigned int nameLength = 0;
		file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
		data.Name.resize(nameLength);
		file.read(data.Name.data(), nameLength);

		file.read(reinterpret_cast<char*>(&data.BoundingBox.MinPoint), sizeof(data.BoundingBox.MinPoint));
		file.read(reinterpret_cast<char*>(&data.BoundingBox.MaxPoint), sizeof(data.BoundingBox.MaxPoint));

		file.read(reinterpret_cast<char*>(&data.Position), sizeof(data.Position));
		file.read(reinterpret_cast<char*>(&data.Rotation), sizeof(data.Rotation));
		file.read(reinterpret_cast<char*>(&data.Scale), sizeof(data.Scale));

		file.read(reinterpret_cast<char*>(&data.VerticesAmount), sizeof(data.VerticesAmount));
		file.read(reinterpret_cast<char*>(&data.VertexElements), sizeof(data.VertexElements));
		file.read(reinterpret_cast<char*>(&data.IndicesAmount), sizeof(data.IndicesAmount));

		file.read(reinterpret_cast<char*>(&data.HasPosition), sizeof(data.HasPosition));
		file.read(reinterpret_cast<char*>(&data.HasNormal), sizeof(data.HasNormal));
		file.read(reinterpret_cast<char*>(&data.HasTexCoord), sizeof(data.HasTexCoord));
		file.read(reinterpret_cast<char*>(&data.HasTangent), sizeof(data.HasTangent));
		file.read(reinterpret_cast<char*>(&data.HasColor), sizeof(data.HasColor));

		file.read(reinterpret_cast<char*>(&data.HasBones), sizeof(data.HasBones));
		
		if (data.HasBones)
		{
			unsigned int skeletonSize;
			file.read(reinterpret_cast<char*>(&skeletonSize), sizeof(skeletonSize));

			data.Skeleton.resize(skeletonSize);

			for (unsigned int i = 0; i < skeletonSize; ++i)
			{
				unsigned int nameLength;
				file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

				data.Skeleton[i].Name.resize(nameLength);
				file.read(data.Skeleton[i].Name.data(), nameLength);

				file.read(reinterpret_cast<char*>(&data.Skeleton[i].ParentIndex), sizeof(int));
				file.read(reinterpret_cast<char*>(&data.Skeleton[i].OffsetMatrix), sizeof(matrix4));
			}
		}

		data.Vertices.resize(data.VerticesAmount * data.VertexElements);

		if (data.HasBones)
			data.Bones.resize(data.VerticesAmount);

		unsigned int floatIndex = 0;

		for (unsigned int v = 0; v < data.VerticesAmount; ++v)
		{
			for (unsigned int e = 0; e < data.VertexElements; ++e)
			{
				file.read(reinterpret_cast<char*>(&data.Vertices[floatIndex++]), sizeof(float));
			}

			if (data.HasBones)
			{
				file.read(reinterpret_cast<char*>(data.Bones[v].IDs), sizeof(int) * 4);
				file.read(reinterpret_cast<char*>(data.Bones[v].Weights), sizeof(float) * 4);
			}
		}

		data.Indices.resize(data.IndicesAmount);
		for (unsigned int i = 0; i < data.IndicesAmount; ++i)
			file.read(reinterpret_cast<char*>(&data.Indices[i]), sizeof(unsigned int));

		file.close();

		if (data.HasBones)
		{
			std::vector<float> interleaved;
			interleaved.reserve(data.VerticesAmount * (data.VertexElements + 8));

			unsigned int srcIndex = 0;

			for (unsigned int v = 0; v < data.VerticesAmount; ++v)
			{
				for (unsigned int e = 0; e < data.VertexElements; ++e)
					interleaved.push_back(data.Vertices[srcIndex++]);

				for (int i = 0; i < 4; ++i)
					interleaved.push_back(static_cast<float>(data.Bones[v].IDs[i]));

				for (int i = 0; i < 4; ++i)
					interleaved.push_back(data.Bones[v].Weights[i]);
			}

			data.VertexElements += 8;
			data.Vertices = std::move(interleaved);
		}

		mesh.m_vbo = std::make_shared<VertexBuffer>(data.Vertices.data(), (unsigned int)(sizeof(float) * data.Vertices.size()));
		mesh.m_ebo = std::make_shared<IndexBuffer>(data.Indices.data(), data.IndicesAmount);
		mesh.m_vao = std::make_shared<VertexArray>();

		BufferLayout& layout = mesh.m_vbo->GetLayout();

		if (data.HasPosition)
			layout.AddLayoutElement(0, GLVariableType::FLOAT, 3, "a_Position");
		if (data.HasTexCoord)
			layout.AddLayoutElement(1, GLVariableType::FLOAT, 2, "a_TexCoord");
		if (data.HasNormal)
			layout.AddLayoutElement(2, GLVariableType::FLOAT, 3, "a_Normal");
		if (data.HasTangent)
			layout.AddLayoutElement(3, GLVariableType::FLOAT, 3, "a_Tangent");
		if (data.HasColor)
			layout.AddLayoutElement(4, GLVariableType::FLOAT, 4, "a_Color");

		if (data.HasBones)
		{
			layout.AddLayoutElement(5, GLVariableType::INT, 4, "a_BoneIDs");
			layout.AddLayoutElement(6, GLVariableType::FLOAT, 4, "a_Weights");
		}

		mesh.m_data = data;
		mesh.m_vao->AddBuffer(mesh.m_vbo.get(), mesh.m_ebo.get());

		Log::Trace("Mesh Loaded -> {0}", filepath.string());
	}

	bool MeshImporter::CheckIfIsModel(const char* path)
	{
		Assimp::Importer importer;
		std::string extension = std::filesystem::path(path).extension().string();

		for (char& c : extension)
		{
			c = std::tolower(static_cast<unsigned char>(c));
		}

		if (!extension.empty() && extension[0] == '.')
			extension = extension.substr(1);

		return importer.IsExtensionSupported(extension);
	}

	void MeshImporter::ProcessNode(void* nodePtr, const void* scenePtr, std::vector<std::string>& outputPaths) {
		auto node = static_cast<const aiNode*>(nodePtr);
		auto scene = static_cast<const aiScene*>(scenePtr);

		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			outputPaths.push_back(ProcessMesh(nodePtr, mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene, outputPaths);
		}
	}

	std::string MeshImporter::ProcessMesh(void* nodePtr, void* meshPtr, const void* scenePtr) {
		auto node = static_cast<const aiNode*>(nodePtr);
		auto mesh = static_cast<const aiMesh*>(meshPtr);
		auto scene = static_cast<const aiScene*>(scenePtr);

		MeshData data{};

		data.Name = node->mName.C_Str();
		unsigned int nameLength = (unsigned int)data.Name.size();


		// Extract translation from matrix
		const aiMatrix4x4& transformMatrix = node->mTransformation;
		
		aiVector3D aiScaling, aiPosition;
		aiQuaternion aiRotation;
		transformMatrix.Decompose(aiScaling, aiRotation, aiPosition);

		data.Position = vec3(aiPosition.x, aiPosition.y, aiPosition.z);
		data.Scale = vec3(aiScaling.x, aiScaling.y, aiScaling.z);
		data.Rotation = quaternion(aiRotation.w, aiRotation.x, aiRotation.y, aiRotation.z);

		// Extract translation from matrix

		data.BoundingBox.MaxPoint = { mesh->mAABB.mMax.x,mesh->mAABB.mMax.y,mesh->mAABB.mMax.z };
		data.BoundingBox.MinPoint = { mesh->mAABB.mMin.x,mesh->mAABB.mMin.y,mesh->mAABB.mMin.z };

		data.VerticesAmount = mesh->mNumVertices;
		data.HasBones = mesh->HasBones();
		data.Bones.resize(data.VerticesAmount);

		data.HasPosition = mesh->mNumVertices > 0;
		data.HasTexCoord = mesh->mTextureCoords[0] != nullptr;
		data.HasNormal = mesh->HasNormals();
		data.HasTangent = mesh->HasTangentsAndBitangents();
		data.HasColor = mesh->HasVertexColors(0);

		data.VertexElements = data.HasPosition ? 3 : 0;
		data.VertexElements += data.HasTexCoord ? 2 : 0;
		data.VertexElements += data.HasNormal ? 3 : 0;
		data.VertexElements += data.HasTangent ? 3 : 0;
		data.VertexElements += data.HasColor ? 4 : 0;


		std::unordered_map<std::string, int> boneMapping;

		if (mesh->HasBones())
		{
			data.HasBones = true;
			data.Bones.resize(data.VerticesAmount);

			data.Skeleton.reserve(mesh->mNumBones);

			for (unsigned int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* aiBone = mesh->mBones[i];

				Bone bone;
				bone.Name = aiBone->mName.C_Str();
				bone.ParentIndex = -1;

				aiMatrix4x4& m = aiBone->mOffsetMatrix;
				bone.OffsetMatrix = matrix4(
					m.a1, m.b1, m.c1, m.d1,
					m.a2, m.b2, m.c2, m.d2,
					m.a3, m.b3, m.c3, m.d3,
					m.a4, m.b4, m.c4, m.d4
				);

				int boneIndex = (int)data.Skeleton.size();
				boneMapping[bone.Name] = boneIndex;

				data.Skeleton.push_back(bone);
			}

			for (unsigned int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* aiBone = mesh->mBones[i];
				int boneIndex = boneMapping[aiBone->mName.C_Str()];

				for (unsigned int w = 0; w < aiBone->mNumWeights; w++)
				{
					int vertexId = aiBone->mWeights[w].mVertexId;
					float weight = aiBone->mWeights[w].mWeight;

					data.Bones[vertexId].AddBoneData(boneIndex, weight);
				}
			}

			for (unsigned int i = 0; i < data.VerticesAmount; ++i)
			{
				float total = 0.0f;
				for (int j = 0; j < MAX_BONE_INFLUENCE; ++j)
					total += data.Bones[i].Weights[j];

				if (total > 0.0f)
					for (int j = 0; j < MAX_BONE_INFLUENCE; ++j)
						data.Bones[i].Weights[j] /= total;
			}

			for (size_t i = 0; i < data.Skeleton.size(); ++i)
			{
				const std::string& boneName = data.Skeleton[i].Name;

				aiNode* boneNode = scene->mRootNode->FindNode(boneName.c_str());
				if (!boneNode || !boneNode->mParent)
					continue;

				std::string parentName = boneNode->mParent->mName.C_Str();

				auto it = boneMapping.find(parentName);
				if (it != boneMapping.end())
					data.Skeleton[i].ParentIndex = it->second;
			}
		}
		else
		{
			data.HasBones = false;
		}

		data.IndicesAmount = 0;
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			data.IndicesAmount += face.mNumIndices;
		}

		///// File Creation
		Project project = Application::GetInstance().m_activeProject;
		UUID id;
		std::filesystem::path locationPath = "Meshes";
		locationPath /= id.Get() + ".mesh";

		std::filesystem::path pathToWrite = project.GetChachePath() / locationPath;

		std::ofstream fs(pathToWrite, std::ios::binary | std::ios::trunc);

		
		fs.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
		fs.write(data.Name.data(), nameLength);

		fs.write(reinterpret_cast<const char*>(&data.BoundingBox.MinPoint), sizeof(data.BoundingBox.MinPoint));
		fs.write(reinterpret_cast<const char*>(&data.BoundingBox.MaxPoint), sizeof(data.BoundingBox.MaxPoint));

		fs.write(reinterpret_cast<const char*>(&data.Position), sizeof(data.Position));
		fs.write(reinterpret_cast<const char*>(&data.Rotation), sizeof(data.Rotation));
		fs.write(reinterpret_cast<const char*>(&data.Scale), sizeof(data.Scale));

		fs.write(reinterpret_cast<const char*>(&data.VerticesAmount), sizeof data.VerticesAmount);
		fs.write(reinterpret_cast<const char*>(&data.VertexElements), sizeof data.VertexElements);

		fs.write(reinterpret_cast<const char*>(&data.IndicesAmount), sizeof data.IndicesAmount);

		fs.write(reinterpret_cast<const char*>(&data.HasPosition), sizeof data.HasPosition);
		fs.write(reinterpret_cast<const char*>(&data.HasNormal), sizeof data.HasNormal);
		fs.write(reinterpret_cast<const char*>(&data.HasTexCoord), sizeof data.HasTexCoord);
		fs.write(reinterpret_cast<const char*>(&data.HasTangent), sizeof data.HasTangent);
		fs.write(reinterpret_cast<const char*>(&data.HasColor), sizeof data.HasColor);

		fs.write(reinterpret_cast<const char*>(&data.HasBones), sizeof data.HasBones);

		// Write skeleton
		if (data.HasBones)
		{
			unsigned int skeletonSize = (unsigned int)data.Skeleton.size();
			fs.write((char*)&skeletonSize, sizeof(skeletonSize));

			for (auto& bone : data.Skeleton)
			{
				unsigned int len = (unsigned int)bone.Name.size();
				fs.write((char*)&len, sizeof(len));
				fs.write(bone.Name.data(), len);

				fs.write((char*)&bone.ParentIndex, sizeof(bone.ParentIndex));
				fs.write((char*)&bone.OffsetMatrix, sizeof(bone.OffsetMatrix));
			}
		}

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			///Position
			fs.write(reinterpret_cast<const char*>(&mesh->mVertices[i].x), sizeof mesh->mVertices[i].x);
			fs.write(reinterpret_cast<const char*>(&mesh->mVertices[i].y), sizeof mesh->mVertices[i].y);
			fs.write(reinterpret_cast<const char*>(&mesh->mVertices[i].z), sizeof mesh->mVertices[i].z);

			///TexCoords
			if (data.HasTexCoord) {
				fs.write(reinterpret_cast<const char*>(&mesh->mTextureCoords[0][i].x), sizeof mesh->mTextureCoords[0][i].x);
				fs.write(reinterpret_cast<const char*>(&mesh->mTextureCoords[0][i].y), sizeof mesh->mTextureCoords[0][i].y);
			}

			///Normals
			if (data.HasNormal) {

				fs.write(reinterpret_cast<const char*>(&mesh->mNormals[i].x), sizeof mesh->mNormals[i].x);
				fs.write(reinterpret_cast<const char*>(&mesh->mNormals[i].y), sizeof mesh->mNormals[i].y);
				fs.write(reinterpret_cast<const char*>(&mesh->mNormals[i].z), sizeof mesh->mNormals[i].z);
			}

			///Tangent
			if (data.HasTangent) {

				fs.write(reinterpret_cast<const char*>(&mesh->mTangents[i].x), sizeof mesh->mTangents[i].x);
				fs.write(reinterpret_cast<const char*>(&mesh->mTangents[i].y), sizeof mesh->mTangents[i].y);
				fs.write(reinterpret_cast<const char*>(&mesh->mTangents[i].z), sizeof mesh->mTangents[i].z);
			}

			///Color
			if (data.HasColor) {
				const aiColor4D& c = mesh->mColors[0][i];

				fs.write(reinterpret_cast<const char*>(&c.r), sizeof c.r);
				fs.write(reinterpret_cast<const char*>(&c.g), sizeof c.g);
				fs.write(reinterpret_cast<const char*>(&c.b), sizeof c.b);
				fs.write(reinterpret_cast<const char*>(&c.a), sizeof c.a);
			}

			if (data.HasBones)
			{
				fs.write(reinterpret_cast<const char*>(data.Bones[i].IDs), sizeof(int) * 4);
				fs.write(reinterpret_cast<const char*>(data.Bones[i].Weights), sizeof(float) * 4);
			}

		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j) {
				fs.write(reinterpret_cast<const char*>(&face.mIndices[j]), sizeof face.mIndices[j]);
			}
		}
		fs.close();


		return locationPath.string();
	}
}