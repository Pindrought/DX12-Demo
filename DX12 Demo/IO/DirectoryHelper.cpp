#include "PCH.h"
#include "DirectoryHelper.h"

bool DirectoryHelper::DirectoryExists(const std::filesystem::path& path)
{
    return std::filesystem::is_directory(path);
}

std::filesystem::path DirectoryHelper::GetParentDirectory(const std::filesystem::path& path)
{
    return path.parent_path();
}

std::filesystem::path DirectoryHelper::GetDirectoryFromPath(const std::filesystem::path& path)
{
    return path.parent_path();
}

std::filesystem::path DirectoryHelper::GetFileFromPath(const std::filesystem::path& path)
{
    return path.filename();
}

std::vector<std::filesystem::path> DirectoryHelper::GetListOfDrives()
{
    char buffer[512]{};

    DWORD length = GetLogicalDriveStringsA(sizeof(buffer), buffer);

    if (length == 0)
    {
        assert(false && "Failed to enumerate logical drives.");
        return {};
    }

    std::vector<std::filesystem::path> drives;

    for (char* drive = buffer; *drive; drive += std::strlen(drive) + 1)
    {
        drives.emplace_back(drive);
    }

    return drives;
}

const std::filesystem::path& DirectoryHelper::GetExecutableDirectory()
{
    static const std::filesystem::path executableDirectory = []()
        {
            wchar_t buffer[MAX_PATH];

            DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

            if (length == 0)
            {
                FATAL_ERROR("Failed to determine executable path.");
            }

            return std::filesystem::path(buffer).parent_path();
        }();

    return executableDirectory;
}

const std::filesystem::path& DirectoryHelper::GetResourcesDirectory()
{
    static const std::filesystem::path resourcesDirectory = []()
        {
            auto path = GetExecutableDirectory() / "MWResources";

            if (DirectoryExists(path))
                return path;

            path = GetExecutableDirectory().parent_path() / "MWResources";

            if (DirectoryExists(path))
                return path;

            FATAL_ERROR(
                "Missing MWResources directory.\n"
                "Checked:\n"
                "  <Executable>/MWResources\n"
                "  <Executable Parent>/MWResources");


            return std::filesystem::path{};
        }();

    return resourcesDirectory;
}

const std::filesystem::path& DirectoryHelper::GetAssetsDirectory()
{
    static const std::filesystem::path assetsDirectory =
        GetResourcesDirectory() / "Assets";

    return assetsDirectory;
}

const std::filesystem::path& DirectoryHelper::GetScriptsDirectory()
{
    static const std::filesystem::path scriptsDirectory =
        GetResourcesDirectory() / "Scripts";

    return scriptsDirectory;
}

const std::filesystem::path& DirectoryHelper::GetUserDirectory()
{
    static const std::filesystem::path userDirectory =
        GetResourcesDirectory() / "User";

    return userDirectory;
}

const std::filesystem::path& DirectoryHelper::GetWebDirectory()
{
    static const std::filesystem::path webDirectory =
        GetResourcesDirectory() / "Web";

    return webDirectory;
}

std::filesystem::path DirectoryHelper::GetUltralightResourcesDirectory()
{
    return GetExecutableDirectory() / "resources";
}