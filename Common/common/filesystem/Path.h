#pragma once
#include "common/Typedefs.h"
#include "common/String.h"
#include <filesystem>

//Todo: Go over each of these functions and try to make them more efficient. Seems to be a lot of unecessary string constructions
namespace Path
{
    [[nodiscard]] string GetFileName(const std::filesystem::path& FullPath);
    [[nodiscard]] string GetFileName(const string& FullPath);

    [[nodiscard]] string GetFileNameNoExtension(const std::filesystem::path& FullPath);
    [[nodiscard]] string GetFileNameNoExtension(const string& FullPath);

    [[nodiscard]] string GetParentDirectory(const std::filesystem::path& FullPath);
    [[nodiscard]] string GetParentDirectory(const string& FullPath);

    [[nodiscard]] string GetExtension(const std::filesystem::path& FullPath);
    [[nodiscard]] string GetExtension(const string& FullPath);
    [[nodiscard]] string GetExtension(const char* FullPath);

    //Note: These are separate functions to avoid the risk of breaking code that use the existing functions
    //      The old ones can be deprecated later once these have been shown to work reliably.
    //Alternative of GetExtension() that works for filenames with multiple extensions strung together. E.g. .vpp_pc.MODMAN.ORIG
    //The base version of GetExtension() would only return .ORIG. This will return .vpp_pc.MODMAN.ORIG
    [[nodiscard]] string GetExtensionMultiLevel(const string& path);
    //Version of GetFilename that works for multilevel extensions
    [[nodiscard]] string GetFileNameNoExtensionMultiLevel(const string& path);

    void CreatePath(const string& fullPath);
}
