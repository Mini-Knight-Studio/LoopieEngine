#include "AssetsExplorerInterface.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Helpers/LoopieHelpers.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Files/DirectoryManager.h"
#include "Loopie/Files/FileDialog.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Resources/Types/Texture.h"

#include "Loopie/Importers/TextureImporter.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include <sstream>

namespace Loopie {
	Event<OnEntityOrFileNotification> AssetsExplorerInterface::s_OnFileSelected;
	std::filesystem::path AssetsExplorerInterface::s_SelectedFile = "";

	AssetsExplorerInterface::AssetsExplorerInterface() {
		
	}
	AssetsExplorerInterface::~AssetsExplorerInterface() {
		Application::GetInstance().m_notifier.RemoveObserver(this);
	}

	void AssetsExplorerInterface::Init()
	{	
		std::vector<std::string> iconsToLoad = {
			"assets/icons/icon_file.png",
			"assets/icons/icon_folder.png",
			"assets/icons/icon_folderFill.png"
		};

		std::vector<Metadata> iconsToLoadMetadatas;
		for (size_t i = 0; i < iconsToLoad.size(); i++)
		{
			Metadata& meta = AssetRegistry::GetOrCreateMetadata(iconsToLoad[i]);
			TextureImporter::ImportImage(iconsToLoad[i], meta);
			iconsToLoadMetadatas.emplace_back(meta);
		}
		
		m_fileIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[0]);
		m_emptyFolderIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[1]);
		m_folderIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[2]);

		Application::GetInstance().m_notifier.AddObserver(this);

		const Project& project = Application::GetInstance().m_activeProject;
		GoToDirectory(project.GetAssetsPath());
	}

	void AssetsExplorerInterface::OnNotify(const EngineNotification& id)
	{
		if (id == EngineNotification::OnAssetRegistryReload)
			PassiveRefresh();
	}

	void AssetsExplorerInterface::Update(const InputEventManager& inputEvent) {
		if (m_focused)
			HotKeysControls(inputEvent);

		if (inputEvent.HasEvent(SDL_EVENT_WINDOW_FOCUS_GAINED))
			Refresh();
	}

	void AssetsExplorerInterface::Render() {
		if (ImGui::Begin("Assets Explorer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)){

			const Project& project = Application::GetInstance().m_activeProject;
			if (project.IsEmpty()) {
				ImGui::End();
				return;
			}

			DrawSearchBar();
			ImGui::Separator();

			ImVec2 avail = ImGui::GetContentRegionAvail();
			float sizeLeft;
			float sizeRight;
			m_Splitter.GetSizes(avail, sizeLeft, sizeRight);

			if (ImGui::BeginChild("AssetsTree", ImVec2(sizeLeft, avail.y), 0, ImGuiWindowFlags_HorizontalScrollbar)) {
				ImGui::Separator();
				DrawDirectoryTree();
			}
			ImGui::EndChild();

			m_Splitter.Place(avail);

			if (ImGui::BeginChild("Assets", ImVec2(sizeRight, avail.y), 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
				ImGui::Separator();
				DrawPathBar();
				ImGui::Separator();

				float footerHeight = ImGui::GetFrameHeightWithSpacing();
				if (ImGui::BeginChild("FilesScrollView", ImVec2(0, -footerHeight), 0, 0)) {
					m_focused = ImGui::IsWindowHovered(ImGuiHoveredFlags_None);

					if (ImGui::BeginPopupContextWindow("HierarchyBackgroundContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
						DrawCreateAssetMenu();
						ImGui::EndPopup();
					}

					//if (m_focused)
						GetExternalFile();
					DrawFolderContent();

					
				}
				ImGui::EndChild();
				ImGui::Separator();
				DrawFooter();
			}	
			ImGui::EndChild();

		}
		ImGui::End();

		if (m_pendingCreateAction)
		{
			ImGui::OpenPopup("Set Name");
		}

		if (InputStringPopup("Set Name"))
		{
			std::string name = m_actionBuffer;
			m_actionBuffer[0] = '\0';

			m_pendingCreateAction(name);
			m_pendingCreateAction = nullptr;

			Refresh();
		}
	}

	void AssetsExplorerInterface::HotKeysControls(const InputEventManager& inputEvent)
	{
		if (s_SelectedFile.empty()) {
			return;
		}

		bool actionDone = false;
		if (inputEvent.GetKeyStatus(SDL_SCANCODE_DELETE) == KeyState::DOWN) {
			DirectoryManager::Delete(s_SelectedFile);
			actionDone = true;
		}

		if (inputEvent.GetKeyWithModifier(SDL_SCANCODE_C, KeyModifier::CTRL)) {
			/// Copy
			actionDone = true;
		}

		if (inputEvent.GetKeyWithModifier(SDL_SCANCODE_V, KeyModifier::CTRL)) {
			/// Paste
			actionDone = true;
		}

		if (inputEvent.GetKeyWithModifier(SDL_SCANCODE_X, KeyModifier::CTRL)) {
			/// Cut
			actionDone = true;
		}

		if(actionDone)
			Refresh();
	}

	void AssetsExplorerInterface::GetExternalFile()
	{
		if (m_isSearching)
			return;

		Application& app = Application::GetInstance();
		InputEventManager& inputEvent = app.GetInputEvent();
		if (!inputEvent.HasFileBeenDropped() && !m_fileDropped)
			return;

		if (m_fileDropped && !m_focused) {
			m_fileDropped = false;
			m_droppedFiles.clear();
			return;
		}
		if (!m_fileDropped) {
			m_fileDropped = true;
			m_droppedFiles.clear();
			for (const char* path : inputEvent.GetDroppedFiles()) {
				m_droppedFiles.emplace_back(path);
			}
			return;
		}

		m_fileDropped = false;

		std::filesystem::path assetsPath = app.m_activeProject.GetAssetsPath();
		std::filesystem::path currentPath = m_currentDirectory;

		for (size_t i = 0; i < m_droppedFiles.size(); i++)
		{
			std::filesystem::path droppedPath = m_droppedFiles[i];

			currentPath = std::filesystem::absolute(currentPath);
			droppedPath = std::filesystem::absolute(droppedPath);

			std::filesystem::path targetPath = currentPath / droppedPath.filename();

			bool isInAssetsFolder = droppedPath.string().rfind(assetsPath.string(), 0) == 0;

			if (isInAssetsFolder) {
				if (droppedPath == currentPath) {
					const Project& project = Application::GetInstance().m_activeProject;
					GoToDirectory(assetsPath);
				}
				DirectoryManager::Move(droppedPath, targetPath);
				DirectoryManager::Move(droppedPath += ".meta", targetPath += ".meta");
			}
			else {
				DirectoryManager::Copy(droppedPath, targetPath);
			}
		}

		m_droppedFiles.clear();


		Refresh(true, true, false);
	}

	void AssetsExplorerInterface::GoToDirectory(const std::filesystem::path& directory, bool removeSearch)
	{
		if (std::filesystem::is_directory(directory)) {
			m_currentDirectory = directory;
		}else
			m_currentDirectory = directory.parent_path();

		const Project& project = Application::GetInstance().m_activeProject;
		m_relativePath = std::filesystem::relative(m_currentDirectory, project.GetAssetsPath());

		if (removeSearch) {
			ClearSearch();
			SelectFile("");
		}

		m_relativePathSteps.clear();
		m_relativePathSteps.push_back(project.GetAssetsPath().filename().string());

		if (m_relativePath != ".") {
			for (auto& p : m_relativePath) {
				m_relativePathSteps.push_back(p.string());
			}
		}

		Refresh(false, true, false);
	}

	void AssetsExplorerInterface::SelectFile(const std::filesystem::path& filePath)
	{
		s_SelectedFile = filePath;

		s_OnFileSelected.Notify(OnEntityOrFileNotification::OnFileSelect);

		if (m_isSearching) {
			GoToDirectory(s_SelectedFile, false);
		}

		m_dirtyFooterText = true;
	}

	void AssetsExplorerInterface::DrawSearchBar()
	{
		float fullWidth = ImGui::GetContentRegionAvail().x;
		float searchBarWidthPercentage = 30.0f;
		float searchBarTotalWidth = fullWidth * (searchBarWidthPercentage / 100.0f);

		float buttonWidth = ImGui::GetFontSize() * 1.5f;
		float spacing = ImGui::GetStyle().ItemSpacing.x;

		float inputWidth = searchBarTotalWidth - (buttonWidth + spacing);

		float cursorX = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(cursorX + fullWidth - searchBarTotalWidth);

		if (ImGui::Button("X", ImVec2(buttonWidth, 0))) {
			ClearSearch();
		}

		ImGui::SameLine(0, spacing);

		ImGui::SetNextItemWidth(inputWidth);
		if (ImGui::InputTextWithHint("##search", "Search files...", m_searchBuffer, sizeof(m_searchBuffer))) {
			m_isSearching = strlen(m_searchBuffer) > 0;
			Refresh(false, false, true);
		}
	}


	void AssetsExplorerInterface::ClearSearch()
	{
		m_isSearching = false;
		m_searchBuffer[0] = '\0';
		Refresh(false, false, true);
	}

	void AssetsExplorerInterface::DrawDirectoryTree()
	{
		if (m_dirtyTreeFolders)
			RebuildTreeFolderCache();

		DrawDirectoryTreeNode(m_cachedTreeFolder, 0);
	}

	void AssetsExplorerInterface::DrawDirectoryTreeNode(const CachedDirectoryTreeNode& node, int indent)
	{
		bool isRoot = (indent == 0);

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

		if (node.IsEmpty)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		else
			flags |= ImGuiTreeNodeFlags_OpenOnArrow;

		if (node.Path == m_currentDirectory)
			flags |= ImGuiTreeNodeFlags_Selected;

		std::string name = node.Path.filename().string();
		if (name.empty())
			name = node.Path.root_name().string();

		bool opened = ImGui::TreeNodeEx(name.c_str(), flags);

		if (!isRoot)
			DragFile(node.Path.string());
		DropFile(node.Path.string());

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			GoToDirectory(node.Path);

		if (opened && !node.IsEmpty)
		{
			for (const auto& child : node.Children)
				DrawDirectoryTreeNode(child, indent + 1);

			ImGui::TreePop();
		}
	}

	void AssetsExplorerInterface::DrawPathBar()
	{
		const Project& project = Application::GetInstance().m_activeProject;

		if (m_isSearching) {
			ImGui::Text("Searching Files...");
			return;
		}

		for (size_t i = 0; i < m_relativePathSteps.size(); ++i)
		{
			if (i != 0) {
				ImGui::SameLine();
				ImGui::Text("/");
				ImGui::SameLine();
			}

			ImVec2 textSize = ImGui::CalcTextSize(m_relativePathSteps[i].c_str());
			if (ImGui::Selectable(m_relativePathSteps[i].c_str(), false, 0, textSize))
			{
				std::filesystem::path newPath = project.GetAssetsPath();
				for (size_t j = 1; j <= i; ++j)
					newPath /= m_relativePathSteps[j];
				GoToDirectory(newPath);
			}
		}
	}

	void AssetsExplorerInterface::DrawFolderContent()
	{
		const Project& project = Application::GetInstance().m_activeProject;

		if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))) {
			SelectFile("");
		}

		if (!DirectoryManager::Contains(m_currentDirectory))
			GoToDirectory(project.GetAssetsPath());

		int cellSize = thumbnailSize + padding;
		float availX = ImGui::GetContentRegionAvail().x;
		int columnCount = std::max(1, (int)(availX / cellSize));

		ImVec2 iconSize = ImVec2((float)thumbnailSize, (float)thumbnailSize);

		ImGui::Columns(columnCount, nullptr, false);

		std::vector<CachedFileNode>* filesToShow;
		
		if (m_isSearching) {
			if (m_dirtyFileSearch)
				RebuildSearchFilesCache();
			filesToShow = &m_cachedSearchFiles;
		}
		else {
			if (m_dirtyFolderFiles)
				RebuildFolderFilesCache();
			filesToShow = &m_cachedFolderFiles;
		}

		for (size_t i = 0; i < filesToShow->size(); ++i) {
			const CachedFileNode& node = (*filesToShow)[i];

			ImGui::PushID((int)i);

			std::shared_ptr<Texture> icon = nullptr;
			if (node.IsDirectory) {
				icon = node.IsEmpty ? m_emptyFolderIcon : m_folderIcon;
			}
			else
				icon = m_fileIcon;

			ImVec2 cursor = ImGui::GetCursorScreenPos();

			ImGui::Image((ImTextureID)icon->GetRendererId(), iconSize);

			if (ImGui::IsItemHovered()) {
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (node.IsDirectory) {
						GoToDirectory(node.Path);
						SelectFile(node.Path);
					}
					else
					{
						OpenFile(node.Path);
					}
				}
				else if (ImGui::IsItemClicked()) {
					SelectFile(node.Path);
				}					
			}

			if (ImGui::BeginPopupContextItem("FileContext", ImGuiPopupFlags_MouseButtonRight)) {
				DrawContextMenu(node.Path);
				ImGui::EndPopup();
			}

			

			DragFile(node.Path.string());
			if (node.IsDirectory)
				DropFile(node.Path.string());

			ImGui::SetCursorScreenPos(cursor);
			ImGuiSelectableFlags flags = ImGuiSelectableFlags_Disabled;
			if (s_SelectedFile == node.Path)
				flags |= ImGuiSelectableFlags_Highlight;
			if (ImGui::Selectable("##file", false, flags, ImVec2((float)thumbnailSize, (float)thumbnailSize))) { }

			ImGuiHelpers::TextCentered(ImGuiHelpers::TruncateText(node.Path.stem().string(), (float)thumbnailSize).c_str(), 0.5f);

			

			ImGui::NextColumn();
			ImGui::PopID();
		}
		ImGui::Columns(1);
	}

	void AssetsExplorerInterface::DrawFooter()
	{
		if (m_dirtyFooterText)
			RebuildFooter();

		if (!s_SelectedFile.empty()) {
			
			ImGui::TextDisabled(m_cachedFooterText.c_str());
		}
	}

	void AssetsExplorerInterface::DragFile(const std::string& from)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceAllowNullID))
		{
			std::string filePathStr = from;
			ImGui::SetDragDropPayload("ASSET_EXPLORER_FILE", filePathStr.c_str(), filePathStr.size() + 1);
			ImGui::EndDragDropSource();
		}
	}
	void AssetsExplorerInterface::DropFile(const std::string& to)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_EXPLORER_FILE"))
			{
				const char* from = (const char*)payload->Data;
				std::filesystem::path fromPath = from;
				std::filesystem::path newPath = to;
				newPath /= fromPath.filename().string();
				if (fromPath == m_currentDirectory) {
					const Project& project = Application::GetInstance().m_activeProject;
					GoToDirectory(newPath);
				}

				DirectoryManager::Move(fromPath, newPath);
				DirectoryManager::Move(fromPath += ".meta", newPath += ".meta");

				Refresh();
			}
			ImGui::EndDragDropTarget();
		}
	}

	void AssetsExplorerInterface::Reload()
	{
		Refresh();
		const Project& project = Application::GetInstance().m_activeProject;
		m_currentDirectory = project.GetAssetsPath();
		ClearSearch();
		SelectFile("");
	}

	void AssetsExplorerInterface::Refresh(bool folderTree, bool folderFiles, bool searchFiles)
	{
		AssetRegistry::RefreshAssetRegistry();
		const Project& project = Application::GetInstance().m_activeProject;

		if (!std::filesystem::exists(m_currentDirectory))
			m_currentDirectory = project.GetAssetsPath();

		if (!std::filesystem::exists(s_SelectedFile))
			s_SelectedFile = "";

		m_dirtyFooterText = true;

		if(!m_dirtyTreeFolders)
			m_dirtyTreeFolders = folderTree;
		if(!m_dirtyFolderFiles)
			m_dirtyFolderFiles = folderFiles;
		if(!m_dirtyFileSearch)
			m_dirtyFileSearch = searchFiles;
	}

	void AssetsExplorerInterface::PassiveRefresh()
	{
		const Project& project = Application::GetInstance().m_activeProject;

		if (!std::filesystem::exists(m_currentDirectory))
			m_currentDirectory = project.GetAssetsPath();

		if (!std::filesystem::exists(s_SelectedFile))
			s_SelectedFile = "";

		m_dirtyFooterText = true;

		if (!m_dirtyTreeFolders)
			m_dirtyTreeFolders = true;
		if (!m_dirtyFolderFiles)
			m_dirtyFolderFiles = true;
		if (!m_dirtyFileSearch)
			m_dirtyFileSearch = true;
	}

	void AssetsExplorerInterface::RebuildTreeFolderCache()
	{
		const Project& project = Application::GetInstance().m_activeProject;
		m_cachedTreeFolder = BuildDirectoryNode(project.GetAssetsPath());
		m_dirtyTreeFolders = false;
	}

	AssetsExplorerInterface::CachedDirectoryTreeNode AssetsExplorerInterface::BuildDirectoryNode(const std::filesystem::path& directory)
	{
		CachedDirectoryTreeNode node;
		node.Path = directory;

		for (auto& entry : std::filesystem::directory_iterator(directory, std::filesystem::directory_options::skip_permission_denied))
		{
			if (entry.is_directory())
			{
				node.IsEmpty = false;
				node.Children.push_back(BuildDirectoryNode(entry.path()));
			}
		}
		return node;
	}

	void AssetsExplorerInterface::RebuildFolderFilesCache()
	{
		m_cachedFolderFiles.clear();
		if (!std::filesystem::exists(m_currentDirectory))
			return;

		for (auto& entry : std::filesystem::directory_iterator(m_currentDirectory, std::filesystem::directory_options::skip_permission_denied))
		{
			if (!MetadataRegistry::IsMetadataFile(entry.path())) {
				CachedFileNode node;
				node.Path = entry.path();
				node.IsDirectory = std::filesystem::is_directory(node.Path); ;
				node.IsEmpty = false;
				if(node.IsDirectory)
					node.IsEmpty = std::filesystem::is_empty(node.Path);
				m_cachedFolderFiles.push_back(node);
			}
		}

		m_dirtyFolderFiles = false;
	}
	void AssetsExplorerInterface::RebuildSearchFilesCache()
	{
		m_cachedSearchFiles.clear();

		const Project& project = Application::GetInstance().m_activeProject;

		std::string query = m_searchBuffer;
		std::transform(query.begin(), query.end(), query.begin(), ::tolower);

		for (auto& entry : std::filesystem::recursive_directory_iterator(project.GetAssetsPath(), std::filesystem::directory_options::skip_permission_denied))
		{
			if (MetadataRegistry::IsMetadataFile(entry.path()))
				continue;

			std::string name = entry.path().filename().string();
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			if (name.find(query) != std::string::npos) {
				CachedFileNode node;
				node.Path = entry.path();
				node.IsDirectory = std::filesystem::is_directory(node.Path); ;
				node.IsEmpty = false;
				if (node.IsDirectory)
					node.IsEmpty = std::filesystem::is_empty(node.Path);
				m_cachedSearchFiles.push_back(node);
			}
		}

		m_dirtyFileSearch = false;
	}
	void AssetsExplorerInterface::RebuildFooter()
	{
		if (!s_SelectedFile.empty()) {
			const Project& project = Application::GetInstance().m_activeProject;
			std::filesystem::path path = project.GetAssetsPath();
			path = path.stem() / std::filesystem::relative(s_SelectedFile, path);

			m_cachedFooterText = path.string();

		}
		else
			m_cachedFooterText = "";
		
		m_dirtyFooterText = false;
	}

	void AssetsExplorerInterface::DrawContextMenu(const std::filesystem::path& file)
	{
		if (ImGui::MenuItem("Copy"))
		{
			
		}

		if (ImGui::MenuItem("Cut"))
		{

		}

		if (ImGui::MenuItem("Delete"))
		{
			DirectoryManager::Delete(file);
			Refresh();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Rename"))
		{
			strcpy(m_actionBuffer, file.stem().string().c_str());

			m_pendingCreateAction = [this, file](const std::string& name)
				{
					std::filesystem::path newPath =
						file.parent_path() / name;

					DirectoryManager::Move(
						file,
						newPath.string() + file.extension().string());

					DirectoryManager::Move(
						file.string() + ".meta",
						newPath.string() + file.extension().string() + ".meta");
				};
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Copy UUID"))
		{
			Metadata* data = AssetRegistry::GetMetadata(file.string());
			if (data)
			{
				Application::GetInstance().m_clipboard.Copy(data->UUID.Get());
			}
		}

		ImGui::Separator();

		if(ImGui::MenuItem("Show in Explorer"))
		{
			FileDialog::OpenInExplorer(file);
		}
	}

	void AssetsExplorerInterface::DrawCreateAssetMenu() {
		if (ImGui::MenuItem("Create Folder"))
		{
			strcpy(m_actionBuffer, "NewFolder");
			m_pendingCreateAction = [this](const std::string& name)
				{
					CreateFolder(m_currentDirectory, name);
				};
		}

		if (ImGui::MenuItem("Create Material"))
		{
			strcpy(m_actionBuffer, "NewMaterial");
			m_pendingCreateAction = [this](const std::string& name)
				{
					CreateMaterial(m_currentDirectory, name);
				};
		}

		if (ImGui::MenuItem("Create Scene"))
		{
			strcpy(m_actionBuffer, "NewScene");
			m_pendingCreateAction = [this](const std::string& name)
				{
					CreateScene(m_currentDirectory, name);
				};
		}

		if (ImGui::MenuItem("Create Script"))
		{
			strcpy(m_actionBuffer, "NewScript");
			m_pendingCreateAction = [this](const std::string& name)
				{
					CreateSript(m_currentDirectory, name);
				};
		}


		ImGui::Separator();

		if (ImGui::MenuItem("Show in Explorer"))
		{
			FileDialog::OpenInExplorer(m_currentDirectory);
		}

	}

	bool AssetsExplorerInterface::InputStringPopup(const char* popupId)
	{
		bool confirmed = false;

		if (ImGui::BeginPopupModal(popupId, nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("##name", m_actionBuffer, sizeof(m_actionBuffer));

			ImGui::Spacing();

			if (ImGui::Button("Confirm", ImVec2(120, 0)))
			{
				confirmed = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				m_pendingAction = PendingAction::None;
				m_pendingCreateAction = nullptr;
				m_actionBuffer[0] = '\0';
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		return confirmed;
	}

	void AssetsExplorerInterface::OpenFile(const std::filesystem::path& filePath)
	{
		if (filePath.extension().string() == ".scene") {
			Application::GetInstance().GetScene().ReadAndLoadSceneFile(filePath.string());
		}
	}

	std::string AssetsExplorerInterface::CreateFolder(const std::filesystem::path& directory, const std::string& name) {
		std::vector<std::string> names;
		Helper::GetPathExistingNames(directory, names);
		std::filesystem::path filePath = DirectoryManager::CreateFolder(m_currentDirectory, Helper::MakeUniqueName(name, names));
		Refresh();
		return filePath.string();
	}

	std::string AssetsExplorerInterface::CreateMaterial(const std::filesystem::path& directory, const std::string& name)
	{

		std::vector<std::string> names;
		Helper::GetPathExistingNames(directory, names);
		std::filesystem::path filePath = directory / Helper::MakeUniqueName(name, names);
		filePath += ".mat";
		if (!DirectoryManager::Contains(filePath)) {
			DirectoryManager::Copy("assets/materials/defaultMaterial.mat", filePath);
			Refresh();
		}
		return filePath.string();
	}
	std::string AssetsExplorerInterface::CreateScene(const std::filesystem::path& directory, const std::string& name)
	{
		std::vector<std::string> names;
		Helper::GetPathExistingNames(directory, names);
		std::filesystem::path filePath = directory / Helper::MakeUniqueName(name, names);
		filePath += ".scene";
		if (!DirectoryManager::Contains(filePath)) {
			DirectoryManager::Copy("assets/scenes/DefaultScene.scene", filePath);
			Refresh();
		}
		return filePath.string();
	}
	std::string AssetsExplorerInterface::CreateSript(const std::filesystem::path& directory, const std::string& name)
	{

		std::vector<std::string> names;
		Helper::GetPathExistingNames(directory, names);

		// Generate unique name
		std::string uniqueName = Helper::MakeUniqueName(name, names);

		std::filesystem::path filePath = directory / uniqueName;
		filePath += ".cs";

		if (!DirectoryManager::Contains(filePath))
		{
			// Copy template
			DirectoryManager::Copy("assets/scripts/DefaultScript.cs", filePath);

			// Read file content
			std::ifstream inFile(filePath);
			std::stringstream buffer;
			buffer << inFile.rdbuf();
			inFile.close();

			std::string content = buffer.str();

			// Replace placeholder with class name
			const std::string placeholder = "{$SCRIPT_NAME}";
			size_t pos = content.find(placeholder);
			if (pos != std::string::npos)
			{
				content.replace(pos, placeholder.length(), uniqueName);
			}

			// Write modified content back
			std::ofstream outFile(filePath);
			outFile << content;
			outFile.close();

			Refresh();
		}

		return filePath.string();
	}
}