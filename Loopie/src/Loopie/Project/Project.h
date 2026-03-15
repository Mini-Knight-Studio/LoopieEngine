#pragma once

#include <filesystem>

namespace Loopie {
	class Project
	{
	public:
		Project() = default;
		~Project() = default;

		bool Create(const std::filesystem::path& projectPath, const std::string& name);
		bool Open(const std::filesystem::path& pathPath);

		const std::string GetProjectName() const { return m_projectPath.filename().string(); }
		const std::string GetNameFromConfig() const { return m_name; }
		bool IsEmpty() const { return m_projectPath.empty() || !std::filesystem::exists(m_projectPath); }
		
		const std::filesystem::path& GetProjectPath() const { return m_projectPath; }
		const std::filesystem::path& GetAssetsPath() const { return m_assetsPath; }
		const std::filesystem::path& GetChachePath() const { return m_cachePath; }
		const std::filesystem::path& GetConfigPath() const { return m_congifPath; }
		const std::filesystem::path& GetGameDLLPath() const { return m_gameDLLPath; }

	private:
		const void CreateDefaultPaths();
		const void CreateProjFiles();

		const void CreateConfigFile(const std::string& projectName, const std::string defaultScenePath);
		const void ReadConfigFile();
	private:
		std::filesystem::path m_projectPath;
		std::filesystem::path m_assetsPath;
		std::filesystem::path m_cachePath;

		std::filesystem::path m_congifPath;
		std::filesystem::path m_gameDLLPath;

		std::string m_name;
	};
}