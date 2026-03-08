#include "EditorMenuInterface.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Window.h"
#include "Loopie/Files/FileDialog.h"
#include "Loopie/Files/DirectoryManager.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Project/ProjectConfig.h"
#include "Loopie/Collisions/CollisionProcessor.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <SDL3/SDL_misc.h>
#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_gpu.h>

namespace Loopie {
	EditorMenuInterface::EditorMenuInterface() {
	}

	void EditorMenuInterface::Render() {

		ImGuiID openProjectPopUpId = ImGui::GetID("###OpenProjectPopUp");
		ImGuiID createProjectPopUpId = ImGui::GetID("###CreateProjectPopUp");
		ImGuiID saveScenePopUpId = ImGui::GetID("###SaveScenePopUp");
		ImGuiID loadScenePopUpId = ImGui::GetID("###LoadScenePopUp");

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					ImGui::OpenPopup(openProjectPopUpId);
					m_projectPath = "";
				}

				if (ImGui::MenuItem("New"))
				{
					ImGui::OpenPopup(createProjectPopUpId);
					m_projectPath = "";
					m_projectName[0] = '\0';
				}

				// *** How Save Scene should work  *** - PSS 08/12/25
				// I believe that when creating a new project a scene should be generated
				// and a default save should be created for it.
				// So saving should never create a pop-up, but instead save on the already
				// existing save file.

				bool existsPath = std::filesystem::exists(Application::GetInstance().GetScene().GetFilePath());
				if (ImGui::MenuItem("Save Scene [WIP]", nullptr, false, existsPath))
				{
					Application::GetInstance().GetScene().SaveScene(Application::GetInstance().GetScene().GetFilePath());
				}

				if (ImGui::MenuItem("Save Scene As [WIP]"))
				{
					ImGui::OpenPopup(saveScenePopUpId);
					m_newScenePath = "";
					m_sceneName[0] = '\0';
				}

				if (ImGui::MenuItem("Load Scene [WIP]"))
				{
					ImGui::OpenPopup(loadScenePopUpId);
					m_newScenePath = "";
				}

				if (ImGui::MenuItem("Exit"))
					Application::GetInstance().Close();

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Disable"))
				{
					Application::GetInstance().SetInterfaceState(false);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Info"))
			{

				if (ImGui::MenuItem("Configuration"))
				{
					m_showInfoConfigMenu = true;
				}

				ImGui::EndMenu();
			}

			// *** PSS - Created this menu for testing features 08/12/25 ***
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Octree Stats Console Information"))
				{
					Application::GetInstance().GetScene().GetOctree().DebugPrintOctreeStatistics();
				}

				if (ImGui::MenuItem("Octree Hierarchy Console Information"))
				{
					Application::GetInstance().GetScene().GetOctree().DebugPrintOctreeHierarchy();
				}

				if (ImGui::MenuItem("Rebuild Octree"))
				{
					Application::GetInstance().GetScene().GetOctree().Rebuild();
				}

				if (ImGui::MenuItem("Reload Last Saved Scene... "))
				{
					Application::GetInstance().GetScene().ReadAndLoadSceneFile(Application::GetInstance().GetScene().GetFilePath());
				}

				if (ImGui::MenuItem("Toggle Octree visualization"))
				{
					Application::GetInstance().GetScene().GetOctree().ToggleShouldDraw();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Collisions"))
			{
				if(ImGui::MenuItem("Collision Matrix"))
				{
					m_showCollisionMatrixMenu = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Documentation"))
				{
					SDL_OpenURL("https://github.com/UPC-GameEngines-BCN-2025/LoopieEngine/docs");
				}

				if (ImGui::MenuItem("Report"))
				{
					SDL_OpenURL("https://github.com/UPC-GameEngines-BCN-2025/LoopieEngine/issues");
				}

				if (ImGui::MenuItem("Release"))
				{
					SDL_OpenURL("https://github.com/UPC-GameEngines-BCN-2025/LoopieEngine/releases");
				}

				if (ImGui::MenuItem("About"))
				{
					m_showAboutMenu = true;
				}
				ImGui::EndMenu();
			}


			ImGui::EndMainMenuBar();
		}

		if (m_showAboutMenu)
			RenderAboutMenu();

		if (m_showInfoConfigMenu)
			RenderInfoConfigMenu();

		if (m_showCollisionMatrixMenu)
			RenderCollisionMatrixMenu();

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		RenderOpenProjectPopUp();

		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		RenderCreateProjectPopUp();

		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		RenderSaveScenePopUp();

		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		RenderLoadScenePopUp();
		
	}

	void EditorMenuInterface::RenderAboutMenu()
	{

		ImGui::Begin("About Loopie Engine", &m_showAboutMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);

		ImGui::Text("Loopie Engine v0.0.1");
		ImGui::Separator();
		ImGui::Text("Developed by:");
		ImGui::BulletText("Ana Alcazar");
		ImGui::BulletText("Pol Sierra");
		ImGui::BulletText("Adria Garcia");

		ImGui::Separator();
		ImGui::Text("Built using:");
		ImGui::BulletText("SDL3 / OpenGl / Glm");
		ImGui::BulletText("Dear ImGui / ImGuizmo");
		ImGui::BulletText("Assimp / Spdlog / Devil");
		ImGui::BulletText("Nativefiledialog-Extended");
		ImGui::BulletText("nlohmann-json");

		ImGui::Separator();
		ImGui::BeginChild("LicenseText",{300,150});
		ImGui::TextWrapped(
			"MIT License\n"
			"Copyright (c) 2025 CITM - UPC\n\n"
			"Permission is hereby granted, free of charge, to any person obtaining a copy "
			"of this software and associated documentation files (the \"Software\"), to deal "
			"in the Software without restriction, including without limitation the rights "
			"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
			"copies of the Software, and to permit persons to whom the Software is "
			"furnished to do so, subject to the following conditions:\n\n"
			"The above copyright notice and this permission notice shall be included in all "
			"copies or substantial portions of the Software.\n\n"
			"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
			"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
			"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
			"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
			"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
			"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
			"SOFTWARE.");
		ImGui::EndChild();

		ImGui::End();
	}

	void EditorMenuInterface::RenderInfoConfigMenu() {
		ImGui::SetNextWindowFocus();

		ImGui::Begin("Configuration", &m_showInfoConfigMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse );

		Window& window = Application::GetInstance().GetWindow();

		if (ImGui::CollapsingHeader("Application")) {
			ImGui::Text("Engine: LoopieEngine");
			ImGui::Text("Organization: LoopieStudios");
		}

		if (ImGui::CollapsingHeader("Window")) {

			bool windowFullscreen = window.IsFullscreen();
			if (ImGui::Checkbox("Fullscreen", &windowFullscreen)) {
				window.SetWindowFullscreen(windowFullscreen);
			}
			bool windowVsync = window.IsVsyncEnabled();
			if (ImGui::Checkbox("VSync", &windowVsync)) {
				window.SetVsync(windowVsync);
			}
			if (!windowVsync) {
				int frameRate = window.GetFramerateLimit();
				int minFrameRate = 10;
				int maxFrameRate = 9999;

				if (ImGui::SliderInt("FrameRate", &frameRate, minFrameRate, maxFrameRate,"%d", ImGuiSliderFlags_Logarithmic)) {
					window.SetFramerateLimit(frameRate);
				}
			}

			if (ImGui::Button("Save Changes")) {
				JsonData data = ProjectConfig::GetData();
				data.SetValue("engine_config.vsync", window.IsVsyncEnabled());
				data.SetValue("engine_config.target_framerate", window.GetFramerateLimit());
				data.SetValue("engine_config.fullscreen", window.IsFullscreen());
				ProjectConfig::Save(data);
			}


			float ms = (float)Time::GetDeltaTimeMs();
			float fps = ms > 0.0f ? 1000.0f / ms : 0.0f;

			if (m_fpsLog.size() >= LOG_SIZE)
				m_fpsLog.erase(m_fpsLog.begin());
			m_fpsLog.push_back(fps);

			if (m_msLog.size() >= LOG_SIZE)
				m_msLog.erase(m_msLog.begin());
			m_msLog.push_back(ms);

			char title[25];
			sprintf_s(title, 25, "Framerate %.1f", m_fpsLog.back());
			ImGui::PlotHistogram("##framerate", &m_fpsLog[0], (int)m_fpsLog.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
			sprintf_s(title, 25, "Milliseconds %.1f", m_msLog.back());
			ImGui::PlotHistogram("##milliseconds", &m_msLog[0], (int)m_msLog.size(), 0, title, 0.0f, 40.0f, ImVec2(310, 100));

		}

		if (ImGui::CollapsingHeader("Hardware Info")) {		
			ImGui::Text("CPUs: %d", SDL_GetNumLogicalCPUCores());
			ImGui::Text("Cache: %dkb", SDL_GetCPUCacheLineSize());

			ImGui::Text("System RAM: %.1f GB", SDL_GetSystemRAM() / 1024.0f);

			std::string caps = "Caps: ";
			if (SDL_HasAltiVec()) caps += "MW,";
			if (SDL_HasMMX()) caps += "MMX,";
			if (SDL_HasSSE()) caps += "SSE,";
			if (SDL_HasSSE2()) caps += "SSE2,";
			if (SDL_HasSSE3()) caps += "SSE3,";
			if (SDL_HasSSE41()) caps += "SSE41,";
			if (SDL_HasSSE42()) caps += "SSE42,";
			if (SDL_HasAVX()) caps += "AVX,";
			if (SDL_HasAVX2()) caps += "AVX2,";
			if (SDL_HasAVX512F()) caps += "AVX512,";

			if (!caps.empty() && caps.back() == ',') caps.pop_back();
			ImGui::Text("%s", caps.c_str());

		}

		ImGui::InvisibleButton("##", { 310,1 });
		ImGui::End();
	}

	void EditorMenuInterface::RenderCollisionMatrixMenu()
	{
		ImGui::Begin("Collision Layers", &m_showCollisionMatrixMenu, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

		auto& layers = Loopie::CollisionProcessor::GetLayers();
		size_t layerCount = layers.size();

		ImGui::Text("Layers:");

		if (ImGui::TreeNode("Layer List"))
		{
			ImGui::BeginChild("LayersScroll", ImVec2(250, 250), true);
			for (size_t i = 0; i < layerCount; ++i)
			{
				ImGui::PushID((int)i);

				ImGuiInputTextFlags flags = (i == 0) ? ImGuiInputTextFlags_ReadOnly : 0;
				std::string name = layers[i].name;

				ImGui::Text("%d:", (int)i);
				ImGui::SameLine();
				if (ImGui::InputText("##LayerName", &name, flags))
				{
					Loopie::CollisionProcessor::SetLayerName((unsigned int)i, name);
				}

				ImGui::PopID();
			}
			ImGui::EndChild();
			ImGui::TreePop();
		}

		ImGui::Separator();

		ImGui::Text("Collision Matrix:");
		if (ImGui::TreeNode("Matrix List"))
		{
			float padding = 4.0f;
			float checkboxSize = 20.0f;

			float firstColWidth = 0.0f;
			for (auto& layer : layers)
			{
				float w = ImGui::CalcTextSize(layer.name.c_str()).x;
				if (w > firstColWidth) firstColWidth = w;
			}
			firstColWidth += padding * 2;

			float maxHeaderHeight = 0.0f;
			for (auto& layer : layers)
			{
				float h = layer.name.size() * ImGui::GetTextLineHeight() + padding * 2;
				if (h > maxHeaderHeight) maxHeaderHeight = h;
			}

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 origin = ImGui::GetCursorScreenPos();

			for (size_t j = 0; j < layerCount; ++j)
			{
				const std::string& name = layers[j].name;
				float totalTextHeight = (name.size() + 1) * ImGui::GetTextLineHeight();

				float x = origin.x + firstColWidth + j * checkboxSize + (checkboxSize - ImGui::CalcTextSize("A").x) / 2.0f;
				float yStart = origin.y + padding + (maxHeaderHeight - totalTextHeight) / 2.0f;

				float y = yStart;
				for (char c : name)
				{
					ImGui::SetCursorScreenPos(ImVec2(x, y));
					ImGui::Text("%c", c);
					y += ImGui::GetTextLineHeight();
				}
			}

			for (size_t i = 0; i < layerCount; ++i)
			{
				ImVec2 rowPos(origin.x, origin.y + maxHeaderHeight + i * checkboxSize);

				draw_list->AddText(ImVec2(rowPos.x + padding, rowPos.y), ImGui::GetColorU32(ImGuiCol_Text), layers[i].name.c_str());
				for (size_t j = 0; j < layerCount; ++j)
				{
					ImVec2 boxPos(origin.x + firstColWidth + j * checkboxSize, origin.y + maxHeaderHeight + i * checkboxSize);
					ImGui::SetCursorScreenPos(boxPos);

					bool collides = Loopie::CollisionProcessor::GetLayerCollision((unsigned int)i, (unsigned int)j);
					if (ImGui::Checkbox(("##" + std::to_string(i) + "_" + std::to_string(j)).c_str(), &collides))
						Loopie::CollisionProcessor::SetLayerCollision((unsigned int)i, (unsigned int)j, collides);
				}
			}

			if(ImGui::Button("Clear"))
				CollisionProcessor::ClearMatrix();

			ImGui::TreePop();
		}


		ImGui::Dummy(ImVec2(0, 15));
		float windowWidth = ImGui::GetContentRegionAvail().x;    
		float buttonWidth = 100.0f;                              
		ImGui::SetCursorPosX((windowWidth) - buttonWidth );
		if (ImGui::Button("Save Changes", ImVec2(buttonWidth, 0)))
		{
			CollisionProcessor::SaveLayers();
		}

		ImGui::End();
	}

	void EditorMenuInterface::RenderOpenProjectPopUp()
	{
		if (ImGui::BeginPopupModal("Open Project###OpenProjectPopUp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputText("Path",& m_projectPath, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("##", { 20,20 }))
			{
				DialogResult result = FileDialog::SelectFolder();
				if (result.Status == DialogResultType::SUCCESS)
				{
					m_projectPath = result.Paths[0].string();
				}
			}

			if (ImGui::Button("Open Project", { 150,20 }))
			{
				if (Application::GetInstance().m_activeProject.Open(m_projectPath)) {
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void EditorMenuInterface::RenderCreateProjectPopUp() {
		if (ImGui::BeginPopupModal("Create Project###CreateProjectPopUp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::InputText("Project Name", m_projectName, IM_ARRAYSIZE(m_projectName));

			ImGui::InputText("Path", &m_projectPath, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("##", { 20,20 }))
			{
				DialogResult result = FileDialog::SelectFolder();
				if (result.Status == DialogResultType::SUCCESS)
				{
					m_projectPath = result.Paths[0].string();
				}
			}

			if (ImGui::Button("Create Project", { 150,20 }))
			{
				if (Application::GetInstance().m_activeProject.Create(m_projectPath, m_projectName)) {
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void EditorMenuInterface::RenderSaveScenePopUp()
	{
		if (ImGui::BeginPopupModal("Save Scene###SaveScenePopUp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::InputText("Scene Name", m_sceneName, IM_ARRAYSIZE(m_sceneName));

			ImGui::InputText("Folder Path", &m_newScenePath, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("##", { 20,20 }))
			{
				DialogResult result = FileDialog::SelectFolder();
				if (result.Status == DialogResultType::SUCCESS)
				{
					m_newScenePath = result.Paths[0].string();
				}
			}

			if (ImGui::Button("Save Scene", { 150,20 }))
			{
				DirectoryManager::CreateFile(m_newScenePath, m_sceneName, ".scene");
				m_newScenePath = m_newScenePath + "\\" + m_sceneName + ".scene";
				Application::GetInstance().GetScene().SetFilePath(m_newScenePath);

				Application::GetInstance().GetScene().SaveScene(Application::GetInstance().GetScene().GetFilePath());

				AssetRegistry::RefreshAssetRegistry();
				ImGui::CloseCurrentPopup();

			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void EditorMenuInterface::RenderLoadScenePopUp()
	{
		if (ImGui::BeginPopupModal("Load Scene###LoadScenePopUp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputText("File Path", &m_newScenePath, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("##", { 20,20 }))
			{
				std::vector<FileFilter> sceneFilter = { {"Scene Files","scene"} };
				DialogResult result = FileDialog::SelectFile(sceneFilter);
				if (result.Status == DialogResultType::SUCCESS)
				{
					m_newScenePath = result.Paths[0].string();
				}
			}

			if (ImGui::Button("Open Scene", { 150,20 }))
			{
				// TODO: Check if new scene is valid - PSS 08/12/25
				Application::GetInstance().GetScene().SetFilePath(m_newScenePath);
				Application::GetInstance().GetScene().ReadAndLoadSceneFile(Application::GetInstance().GetScene().GetFilePath());
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}