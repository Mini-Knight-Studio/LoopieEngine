#include "Project.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Files/DirectoryManager.h"
#include "Loopie/Files/Json.h"
#include "Loopie/Project/ProjectConfig.h"

#include "Loopie/Core/Window.h"

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

		std::filesystem::path scenePath = DirectoryManager::CreateFolder(m_assetsPath, "Scenes");
		DirectoryManager::Copy("assets/scenes/DefaultScene.scene", scenePath/"DefaultScene.scene");

		CreateConfigFile(name, (scenePath / "DefaultScene.scene").string());

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
		ReadConfigFile();

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
	const void Project::CreateConfigFile(const std::string& projectName, const std::string defaultScenePath)
	{
		JsonData configData;
		configData.CreateField<std::string>("project_name", projectName);
		configData.CreateField<std::string>("last_scene", defaultScenePath);
		configData.CreateField<std::string>("build_scene", "none");
		JsonNode config = configData.CreateObjectField("engine_config");
		config.CreateField<bool>("vsync", true);
		config.CreateField<bool>("fullscreen", false);
		config.CreateField<int>("target_framerate", 60);
		config = configData.CreateObjectField("editor_config");
		ProjectConfig::Save(configData);

		m_name = projectName;
	}
	const void Project::ReadConfigFile()
	{
		JsonData configData = Json::ReadFromFile(m_congifPath);
		if (!configData.HasKey("", "project_name"))
			configData.CreateField<std::string>("project_name", m_projectPath.filename().string());
		if (!configData.HasKey("", "last_scene"))
			configData.CreateField<std::string>("last_scene", "none");
		if (!configData.HasKey("", "build_scene"))
			configData.CreateField<std::string>("build_scene", "none");

		JsonNode config;
		if (!configData.HasKey("", "engine_config"))
			config = configData.CreateObjectField("engine_config");
		else
			config = configData.Child("engine_config");

		if (!config.HasKey("vsync"))
			config.CreateField<bool>("vsync", true);
		if (!config.HasKey("fullscreen"))
			config.CreateField<bool>("fullscreen", false);
		if (!config.HasKey("target_framerate"))
			config.CreateField<int>("target_framerate", 60);

		if (!configData.HasKey("", "editor_config"))
			config = configData.CreateObjectField("editor_config");
		else
			config = configData.Child("editor_config");


		ProjectConfig::Save(configData);



		Window& window = Application::GetInstance().GetWindow();
		bool vsync = configData.GetValue<bool>("engine_config.vsync", true).Result;
		bool fullscreen = configData.GetValue<bool>("engine_config.fullscreen", false).Result;
		int targetFramerate = configData.GetValue<int>("engine_config.target_framerate", 60).Result;
		std::string name = configData.GetValue<std::string>("project_name", m_projectPath.filename().string()).Result;

		window.SetVsync(vsync);
		window.SetWindowFullscreen(fullscreen);
		window.SetFramerateLimit(targetFramerate);

		window.SetTitle(name.c_str());
		m_name = name;
	}
}