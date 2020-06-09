#pragma once
#include "common/Typedefs.h"
#include <string>
#include <vector>
#include <span>

namespace File
{
    [[nodiscard]] std::vector<char> ReadAllBytes(const std::string& filePath);
    [[nodiscard]] std::string ReadToString(const string& path);
    void WriteToFile(const string& path, std::span<u8> data);
}


