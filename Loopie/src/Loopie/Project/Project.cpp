#include "Project.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Files/DirectoryManager.h"
#include "Loopie/Files/Json.h"

#include <fstream>
#include <sstream>

namespace Loopie {
	bool Project::Create(const std::filesystem::path& projectPath, const std::string& name) {

		if (projectPath.empty() || DirectoryManager::Contains(projectPath / name)) {
			return false;
		}

		m_projectPath = DirectoryManager::CreateFolder(projectPath, name);


		m_congifPath = DirectoryManager::CreateFile(m_projectPath, "project", ".config");
		m_gameDLLPath = m_projectPath / "Scripting/Game.dll";
		CreateDefaultPaths();

		JsonData configData;	
		std::filesystem::path scenePath = DirectoryManager::CreateFolder(m_assetsPath, "Scenes");
		DirectoryManager::Copy("assets/scenes/DefaultScene.scene", scenePath/"DefaultScene.scene");
		//configData.CreateField<std::string>("last_scene", (scenePath / "DefaultScene.scene").string());
		configData.CreateField<std::string>("last_scene", "none");
		Json::WriteToFileFromData(m_congifPath, configData, 4);

		/// Maybe some config Files???? Once Scene Exists a default One
		Application::GetInstance().m_notifier.Notify(EngineNotification::OnProjectChange);
		return true;
	}

	bool Project::Open(const std::filesystem::path& projectPath) {
		if (projectPath.empty()|| !DirectoryManager::Contains(projectPath) || !DirectoryManager::Contains(projectPath / "project.config")) {
			return false;
		}

		m_projectPath = projectPath;
		m_congifPath = projectPath / "project.config";
		m_gameDLLPath = m_projectPath / "Scripting/Game.dll";
		CreateDefaultPaths();

		/// Maybe read/save config Files????

		Application::GetInstance().m_notifier.Notify(EngineNotification::OnProjectChange);
		return true;
	}
	const void Project::CreateDefaultPaths()
	{
		m_assetsPath = DirectoryManager::CreateFolder(m_projectPath, "Assets");

		m_cachePath = DirectoryManager::CreateFolder(m_projectPath, "Library");
		DirectoryManager::CreateFolder(m_cachePath, "Textures");
		DirectoryManager::CreateFolder(m_cachePath, "Meshes");
		DirectoryManager::CreateFolder(m_cachePath, "Materials");
		DirectoryManager::CreateFolder(m_cachePath, "Shaders");

		CreateProjFiles();

	}

	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
		return str;
	}

	const void Project::CreateProjFiles() {
		std::string csProjString = R"(
			<Project Sdk="Microsoft.NET.Sdk">
				<PropertyGroup>
					<TargetFramework>net472</TargetFramework>
					<LangVersion>7.3</LangVersion>
					<Nullable>disable</Nullable>
					<ImplicitUsings>disable</ImplicitUsings>
					<NuGetAudit>false</NuGetAudit>
				</PropertyGroup>

				<ItemGroup>
					<Reference Include="Loopie">
						<HintPath>ENGINE_DLL_PATH</HintPath>
					</Reference>
				</ItemGroup>

			</Project>

			)";

		std::filesystem::path csprojPath = DirectoryManager::CreateFile(m_projectPath, "LoopieProject", ".csproj");
		
		std::ofstream file(csprojPath.string());
		if (!file.is_open())
		{
			Log::Error("CSPROJ not created: {0}", csprojPath.string());
			return;
		}

		std::filesystem::path relative = "../Loopie/Loopie.Core.dll";

		csProjString = ReplaceAll(csProjString, "ENGINE_DLL_PATH", std::filesystem::absolute(relative).string());

		file << csProjString;
		file.close();
	}
}