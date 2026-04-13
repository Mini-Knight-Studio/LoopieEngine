#include "DirectoryManager.h"

#include "Loopie/Core/Log.h"

#include <fstream>

namespace Loopie {
    std::filesystem::path DirectoryManager::CreateFolder(const std::filesystem::path& path, const std::string& folderName)
    {
        if(folderName.empty())
            return std::filesystem::path();

        std::filesystem::path directorypath = path / folderName;
        if (Contains(directorypath)) {
            return directorypath;
        }

        if (std::filesystem::create_directories(directorypath)) {
            return directorypath;
        }
        return std::filesystem::path();
    }

    std::filesystem::path DirectoryManager::CreateFile(const std::filesystem::path& path, const std::string& fileName, const std::string& fileExtension)
    {

        if (fileName.empty())
            return std::filesystem::path();

        std::filesystem::path filePath = path / (fileName + fileExtension);
        if (Contains(filePath)) {
            return filePath;
        }

        std::ofstream ofs(filePath);
        if (ofs.good()) {
            ofs.close();
            return  filePath;
        }
        return std::filesystem::path();
    }

    bool DirectoryManager::Move(const std::filesystem::path& from, const std::filesystem::path& to)
    {
        if (!Contains(from))
            return false;
        if (from == to)
            return false;
        if (std::filesystem::is_directory(from))
        {
            auto relative = std::filesystem::relative(to, from);
            if (!relative.empty() && relative.native()[0] != '.')
                return false;
        }

        std::filesystem::rename(from, to);
        return true;
    }

    bool DirectoryManager::Copy(const std::filesystem::path& fileToCopy, const std::filesystem::path& to)
    {
        if (!Contains(fileToCopy))
            return false;
        if (fileToCopy == to)
            return false;
        if (std::filesystem::is_directory(fileToCopy))
        {
            auto relative = std::filesystem::relative(to, fileToCopy);
            if (!relative.empty() && relative.native()[0] != '.')
                return false;
        }

        std::filesystem::copy(fileToCopy, to, std::filesystem::copy_options::overwrite_existing);
        return true;
    }

    bool DirectoryManager::Delete(const std::filesystem::path& fileToDelete)
    {
        if (!Contains(fileToDelete))
            return false;

        std::error_code ec;
        if (std::filesystem::is_directory(fileToDelete))
            std::filesystem::remove_all(fileToDelete, ec);
        else
            std::filesystem::remove(fileToDelete, ec);

        if (ec) {
            Log::Error("Error deleting: {0}", ec.message().c_str());
            return false;
        }
        return true;
    }

    bool DirectoryManager::Contains(const std::filesystem::path& path)
    {
        return std::filesystem::exists(path);
    }

    bool DirectoryManager::Contains(const std::filesystem::path& path, const std::string& nameToFind)
    {
        return Contains(path/nameToFind);
    }
    
}