#include "Path.h"

namespace Path
{
    string GetFileName(const std::filesystem::path& FullPath)
    {
        return GetFileName(FullPath.string());
    }

    string GetFileName(const string& FullPath)
    {
        if (!std::filesystem::path(FullPath).has_filename())
            return {};

        return std::filesystem::path(FullPath).filename().string();
    }

    string GetFileNameNoExtension(const std::filesystem::path& FullPath)
    {
        return GetFileName(FullPath.string());
    }

    string GetFileNameNoExtension(const string& FullPath)
    {
        if (!std::filesystem::path(FullPath).has_filename())
            return {};

        return std::filesystem::path(FullPath).filename().replace_extension("").string();
    }

    string GetParentDirectory(const std::filesystem::path& FullPath)
    {
        return GetParentDirectory(FullPath.string());
    }

    string GetParentDirectory(const string& FullPath)
    {
        if (!std::filesystem::path(FullPath).has_parent_path())
            return {};

        return std::filesystem::path(FullPath).parent_path().string() + "\\";
    }

    string GetExtension(const std::filesystem::path& FullPath)
    {
        return GetExtension(FullPath.string());
    }

    string GetExtension(const string& FullPath)
    {
        if (!std::filesystem::path(FullPath).has_extension())
            return {};

        return std::filesystem::path(FullPath).extension().string();
    }

    void CreatePath(const string& fullPath)
    {
        if (!std::filesystem::exists(fullPath))
            std::filesystem::create_directories(fullPath);
    }
}
