#pragma once
#include "common/Typedefs.h"
#include "common/String.h"
#include <vector>
#include <span>

namespace File
{
    [[nodiscard]] std::vector<u8> ReadAllBytes(const std::string& filePath);
    [[nodiscard]] std::string ReadToString(const std::string& path);
    void WriteToFile(const std::string& path, std::span<u8> data);
}


