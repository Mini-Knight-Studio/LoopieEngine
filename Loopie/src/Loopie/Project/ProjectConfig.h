#pragma once
#include "Loopie/Project/Project.h"
#include "Loopie/Files/Json.h"
#include <filesystem>

namespace Loopie {
	class ProjectConfig
	{
	public:
		ProjectConfig() = default;
		~ProjectConfig() = default;

		static void SetProject(Project& project) { m_project = &project; }
		static JsonData GetData();
		static void Save(const JsonData& saveData);
	private:
		static Project* m_project;
	};
}