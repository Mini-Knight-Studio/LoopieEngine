#pragma once 

#include "Loopie/Resources/Types/Texture.h"
#include "Editor/Interfaces/Interface.h"
#include "Editor/ImGuiHelpers/ImGuiHelpers.h"

#include "Loopie/Events/Event.h"
#include "Loopie/Events/EventTypes.h"
#include "Editor/Events/EditorEventTypes.h"

#include <filesystem>
#include <vector>
#include <memory>
#include <string>
#include <functional>


namespace Loopie {
	class AssetsExplorerInterface : public Interface , public IObserver<EngineNotification>{
	public:
		AssetsExplorerInterface();
		~AssetsExplorerInterface();

		void Init() override;
		void Update(const InputEventManager& inputEvent) override;
		void Render() override;

		void Reload();

	private:
		struct CachedDirectoryTreeNode
		{
			std::filesystem::path Path;
			bool IsEmpty = true;
			std::vector<CachedDirectoryTreeNode> Children;
		};

		struct CachedFileNode {
			std::filesystem::path Path;
			bool IsDirectory;
			bool IsEmpty;
		};

		enum class PendingAction
		{
			None,
			Rename,
			Create

		};

		void OnNotify(const EngineNotification& id) override;

		void HotKeysControls(const InputEventManager& inputEvent);

		void GetExternalFile();
		void GoToDirectory(const std::filesystem::path& directory, bool removeSearch = true);
		void SelectFile(const std::filesystem::path& filePath);
		void DrawSearchBar();
		void ClearSearch();
		void DrawDirectoryTree();
		void DrawDirectoryTreeNode(const CachedDirectoryTreeNode& node, int indent = 0);
		void DrawPathBar();
		void DrawFolderContent();
		void DrawFooter();

		void DragFile(const std::string& from);
		void DropFile(const std::string& to);

		void Refresh(bool folderTree = true, bool folderFiles = true, bool searchFiles = true, bool reloadRegistry = true);
		void PassiveRefresh();
		void RebuildTreeFolderCache();
		CachedDirectoryTreeNode BuildDirectoryNode(const std::filesystem::path& directory);
		void RebuildFolderFilesCache();
		void RebuildSearchFilesCache();

		void RebuildFooter();

		void DrawContextMenu(const std::filesystem::path& file);
		void DrawCreateAssetMenu();

		bool InputStringPopup(const char* popupId);

		void OpenFile(const std::filesystem::path& filePath);
		std::string CreateFolder(const std::filesystem::path& directory, const std::string& name);
		std::string CreateMaterial(const std::filesystem::path& directory, const std::string& name);
		std::string CreateScene(const std::filesystem::path& directory, const std::string& name);
		std::string CreateSript(const std::filesystem::path& directory, const std::string& name);

	
	public:
		static Event<OnEntityOrFileNotification> s_OnFileSelected;

		static std::filesystem::path s_SelectedFile;
	private:
		std::filesystem::path m_currentDirectory;


		std::filesystem::path m_relativePath;
		std::vector<std::string> m_relativePathSteps;

		char m_searchBuffer[256] = { 0 };
		bool m_isSearching = false;

		int thumbnailSize = 64;
		int padding = 16;

		Splitter m_Splitter{ SplitterMode::VERTICAL, 0.15f, 5.0f, 3.0f, ImGuiCol_ButtonActive };

		std::shared_ptr<Texture> m_fileIcon;
		std::shared_ptr<Texture> m_emptyFolderIcon;
		std::shared_ptr<Texture> m_folderIcon;

		bool m_dirtyTreeFolders = true;
		bool m_dirtyFolderFiles = true;
		bool m_dirtyFileSearch = true;

		CachedDirectoryTreeNode m_cachedTreeFolder;
		std::vector<CachedFileNode> m_cachedFolderFiles;
		std::vector<CachedFileNode> m_cachedSearchFiles;

		bool m_dirtyFooterText = true;
		std::string m_cachedFooterText;
		
		bool m_fileDropped = false;
		std::vector<std::string> m_droppedFiles;


		char m_actionBuffer[256] = { 0 };
		PendingAction m_pendingAction = PendingAction::None;
		std::function<void(const std::string&)> m_pendingCreateAction;
		std::filesystem::path m_pendingDirectory;
		
	};
}