#pragma once
#include "common/Typedefs.h"
#include <string>
#include <vector>

namespace File
{
    [[nodiscard]] std::vector<char> ReadAllBytes(const std::string& filePath);
    [[nodiscard]] std::string ReadToString(const string& path);
}


