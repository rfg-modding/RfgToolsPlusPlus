#pragma once
#include "common/Typedefs.h"
#include "common/String.h"
#include <vector>
#include <span>

namespace File
{
    //Read file to an in memory buffer
    [[nodiscard]] std::vector<u8> ReadAllBytes(const std::string& filePath);
    //Same as ::ReadAllBytes() except the caller takes ownership. Used by RFG format readers to avoid unnecessary copies.
    [[nodiscard]] std::span<u8> ReadAllBytesSpan(const std::string& filePath);
    [[nodiscard]] std::string ReadToString(const std::string& path);
    void WriteToFile(const std::string& path, std::span<u8> data);
    void WriteTextToFile(const std::string& path, std::string_view data);
}


