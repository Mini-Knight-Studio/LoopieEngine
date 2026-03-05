#include "MaterialImporter.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Files/Json.h"

#include "Loopie/Resources/ResourceManager.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem> // Used for checking the extension

namespace Loopie {

	static std::vector<float> ParseNumberList(const std::string& s) {
		std::vector<float> out;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, ',')) {
			out.push_back(std::stof(item));
		}
		return out;
	}

	static glm::vec2 ParseNumberListToVec2(const std::string& s) {
		auto nums = ParseNumberList(s);
		if (nums.size() >= 2)
			return glm::vec2(nums[0], nums[1]);
		return glm::vec2(0.0f);
	}

	static glm::vec3 ParseNumberListToVec3(const std::string& s) {
		auto nums = ParseNumberList(s);
		if (nums.size() >= 3)
			return glm::vec3(nums[0], nums[1], nums[2]);
		return glm::vec3(0.0f);
	}

	static glm::vec4 ParseNumberListToVec4(const std::string& s) {
		auto nums = ParseNumberList(s);
		if (nums.size() >= 4)
			return glm::vec4(nums[0], nums[1], nums[2], nums[3]);
		return glm::vec4(0.0f);
	}

	static glm::mat2 ParseNumberListToMat2(const std::string& s) {
		auto nums = ParseNumberList(s);
		glm::mat2 m(1.0f);
		if (nums.size() >= 4) {
			m[0][0] = nums[0]; m[0][1] = nums[1];
			m[1][0] = nums[2]; m[1][1] = nums[3];
		}
		return m;
	}

	static glm::mat3 ParseNumberListToMat3(const std::string& s) {
		auto nums = ParseNumberList(s);
		glm::mat3 m(1.0f);
		if (nums.size() >= 9) {
			m[0][0] = nums[0]; m[0][1] = nums[1]; m[0][2] = nums[2];
			m[1][0] = nums[3]; m[1][1] = nums[4]; m[1][2] = nums[5];
			m[2][0] = nums[6]; m[2][1] = nums[7]; m[2][2] = nums[8];
		}
		return m;
	}

	static glm::mat4 ParseNumberListToMat4(const std::string& s) {
		auto nums = ParseNumberList(s);
		glm::mat4 m(1.0f);
		if (nums.size() >= 16) {
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = nums[i * 4 + j];
		}
		return m;
	}

	void MaterialImporter::ImportMaterial(const std::string& filepath, Metadata& metadata) {
		if (metadata.HasCache && !metadata.IsOutdated)
			return;

		if (!std::filesystem::exists(filepath)) {
			Log::Error("Material file not found: {0}", filepath);
			return;
		}

		Project project = Application::GetInstance().m_activeProject;

		UUID id;
		std::filesystem::path extension = std::filesystem::path(filepath).extension();
		std::filesystem::path locationPath = "Materials";
		locationPath /= id.Get() + extension.string();

		std::filesystem::path destination = project.GetChachePath() / locationPath;

		std::filesystem::create_directories(destination.parent_path());

		bool success = std::filesystem::copy_file(filepath, destination, std::filesystem::copy_options::overwrite_existing);
		if(!success){
			Log::Error("Failed to copy material to cache: {0}", filepath);
			return;
		}

		metadata.HasCache = true;
		metadata.CachesPath.clear();
		metadata.CachesPath.push_back(locationPath.string());
		metadata.Type = ResourceType::MATERIAL;

		MetadataRegistry::SaveMetadata(filepath, metadata);

		Log::Trace("Material imported -> {0}", filepath);

	}

	void MaterialImporter::LoadMaterial(const std::string& path, Material& material) {
		Project project = Application::GetInstance().m_activeProject;
		std::filesystem::path filepath = project.GetChachePath() / path;
		if (!std::filesystem::exists(filepath)) {
			Log::Warn("Material cache file not found: {0}", filepath.string());
			return;
		}

		JsonData jsonData = Json::ReadFromFile(filepath.string());
		if (jsonData.IsEmpty()) {
			Log::Warn("Failed to parse material JSON: {0}", filepath.string());
			return;
		}

		// Shader UUID
		JsonResult<std::string> shaderNode = jsonData.GetValue<std::string>("shader");
		if (!shaderNode.Found) {
			Log::Warn("Material missing 'shader' field: {0}", filepath.string());
			return;
		}
		// UUID shaderUUID = UUID(shaderNode.Result);

		JsonNode texturesNode = jsonData.Child("textures");
		for (const auto& key : texturesNode.GetObjectKeys())
		{
			JsonResult<std::string> texUUID = texturesNode.GetValue<std::string>(key);
			if (!texUUID.Found)
				continue;
			UUID textureUUID(texUUID.Result);
			Metadata* meta = AssetRegistry::GetMetadata(textureUUID);
			if (!meta)
				continue;
			auto texture = ResourceManager::GetTexture(*meta);
			material.SetTexture(key, texture);
		}

		// Properties
		JsonNode propertiesNode = jsonData.Child("properties");
		std::vector<std::string> propKeys = propertiesNode.GetObjectKeys();

		for (const auto& key : propKeys) {
			JsonNode propNode = propertiesNode.Child(key);
			JsonResult<std::string> typeResult = propNode.GetValue<std::string>("type");
			JsonResult<std::string> valueResult = propNode.GetValue<std::string>("value");

			if (!typeResult.Found || !valueResult.Found)
				continue;

			const std::string& type = typeResult.Result;
			const std::string& value = valueResult.Result;

			UniformValue uv{};

			if (type == "Int") {
				uv.type = UniformType_int;
				uv.value = std::stoi(value);
			}
			else if (type == "Float") {
				uv.type = UniformType_float;
				uv.value = std::stof(value);
			}
			else if (type == "UInt") {
				uv.type = UniformType_uint;
				uv.value = static_cast<unsigned int>(std::stoul(value));
			}
			else if (type == "Bool") {
				uv.type = UniformType_bool;
				uv.value = (value == "True");
			}
			else if (type == "Vec2") {
				uv.type = UniformType_vec2;
				uv.value = ParseNumberListToVec2(value);
			}
			else if (type == "Vec3") {
				uv.type = UniformType_vec3;
				uv.value = ParseNumberListToVec3(value);
			}
			else if (type == "Vec4") {
				uv.type = UniformType_vec4;
				uv.value = ParseNumberListToVec4(value);
			}
			else if (type == "Mat2") {
				uv.type = UniformType_mat2;
				uv.value = ParseNumberListToMat2(value);
			}
			else if (type == "Mat3") {
				uv.type = UniformType_mat3;
				uv.value = ParseNumberListToMat3(value);
			}
			else if (type == "Mat4") {
				uv.type = UniformType_mat4;
				uv.value = ParseNumberListToMat4(value);
			}
			else if (type == "Sampler2D") {
				uv.type = UniformType_Sampler2D;
			}
			else if (type == "Sampler3D") {
				uv.type = UniformType_Sampler3D;
			}
			else if (type == "SamplerCube") {
				uv.type = UniformType_SamplerCube;
			}

			material.SetShaderVariable(key, uv);
		}
	}

	void MaterialImporter::SaveMaterial(const std::string& filepath, Material& material, Metadata& metadata)
	{
		JsonData jsonData;

		Shader& shader = material.GetShader();
		UUID randomUUID;
		//std::string shaderUUIDString = shader.GetUUID().Get();

		jsonData.CreateField("shader", randomUUID.Get());

		JsonNode texturesNode = jsonData.CreateObjectField("textures");
		for (const auto& [name, texture] : material.GetTextures())
		{
			if (!texture)
				continue;
			texturesNode.CreateField(name, texture->GetUUID().Get());
		}

		JsonNode propertiesNode = jsonData.CreateObjectField("properties");

		const auto& props = material.GetUniforms();
		for (const auto& [id, uniformValue] : props)
		{
			std::string typeString;
			std::string valueString;

			switch (uniformValue.type)
			{
			case UniformType_int:
			{
				typeString = "Int";
				int v = std::get<int>(uniformValue.value);
				valueString = std::to_string(v);
				break;
			}
			case UniformType_float:
			{
				typeString = "Float";
				float v = std::get<float>(uniformValue.value);
				valueString = std::to_string(v);
				break;
			}
			case UniformType_uint:
			{
				typeString = "UInt";
				unsigned int v = std::get<unsigned int>(uniformValue.value);
				valueString = std::to_string(v);
				break;
			}
			case UniformType_bool:
			{
				typeString = "Bool";
				bool v = std::get<bool>(uniformValue.value);
				valueString = v ? "True" : "False";
				break;
			}
			case UniformType_vec2:
			{
				typeString = "Vec2";
				auto v = std::get<glm::vec2>(uniformValue.value);
				valueString = GLMVectorToString(v);
				break;
			}
			case UniformType_vec3:
			{
				typeString = "Vec3";
				auto v = std::get<glm::vec3>(uniformValue.value);
				valueString = GLMVectorToString(v);
				break;
			}
			case UniformType_vec4:
			{
				typeString = "Vec4";
				auto v = std::get<glm::vec4>(uniformValue.value);
				valueString = GLMVectorToString(v);

				break;
			}
			case UniformType_mat2:
			{
				typeString = "Mat2";
				auto m = std::get<glm::mat2>(uniformValue.value);
				valueString = GLMMatrixToString(m);
				break;
			}

			case UniformType_mat3:
			{
				typeString = "Mat3";
				auto m = std::get<glm::mat3>(uniformValue.value);
				valueString = GLMMatrixToString(m);

				break;
			}

			case UniformType_mat4:
			{
				typeString = "Mat4";
				auto m = std::get<glm::mat4>(uniformValue.value);
				valueString = GLMMatrixToString(m);
				break;
			}

			case UniformType_Sampler2D:
			case UniformType_Sampler3D:
			case UniformType_SamplerCube:
			{
				typeString = (uniformValue.type == UniformType_Sampler2D ? "Sampler2D" : uniformValue.type == UniformType_Sampler3D ? "Sampler3D" : "SamplerCube");

				//UUID texID = std::get<UUID>(uniformValue.value);
				//valueString = texID.Get();
				break;
			}
			default:
				break;

				
			}
			JsonNode propertyNode = propertiesNode.CreateObjectField(id);
			propertyNode.CreateField("type", typeString);
			propertyNode.CreateField("value", valueString);
		}

		jsonData.ToFile(filepath);
		metadata.IsOutdated = true;
	}

	bool MaterialImporter::CheckIfIsMaterial(const char* path) {
		std::string extension = std::filesystem::path(path).extension().string();
		for (char& c : extension)
		{
			c = std::tolower(static_cast<unsigned char>(c));
		}

		if (!extension.empty() && extension[0] == '.')
			extension = extension.substr(1);

		return extension == "mat";
	}
}