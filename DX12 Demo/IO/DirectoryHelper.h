#pragma once

#include <filesystem>
#include <vector>

class DirectoryHelper
{
public:
    // Filesystem helpers
    static bool DirectoryExists(const std::filesystem::path& path);

    static std::filesystem::path GetParentDirectory(const std::filesystem::path& path);
    static std::filesystem::path GetDirectoryFromPath(const std::filesystem::path& path);
    static std::filesystem::path GetFileFromPath(const std::filesystem::path& path);

    static std::vector<std::filesystem::path> GetListOfDrives();

    // Engine directories
    static const std::filesystem::path& GetExecutableDirectory();
    static const std::filesystem::path& GetResourcesDirectory();
    static const std::filesystem::path& GetAssetsDirectory();
    static const std::filesystem::path& GetScriptsDirectory();
    static const std::filesystem::path& GetUserDirectory();
    static const std::filesystem::path& GetWebDirectory();

    // Ultralight still expects this beside the executable
    static std::filesystem::path GetUltralightResourcesDirectory();
};