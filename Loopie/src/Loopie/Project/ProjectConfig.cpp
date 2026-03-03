#include "ProjectConfig.h"

namespace Loopie
{
	Project* ProjectConfig::m_project;

	JsonData ProjectConfig::GetData() {
		JsonData configData;
		configData = Json::ReadFromFile(m_project->GetConfigPath());
		return configData;
	}

	void ProjectConfig::Save(const JsonData& saveData) {
		Json::WriteToFileFromData(m_project->GetConfigPath(), saveData, 4);
	}
}