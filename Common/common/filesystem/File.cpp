#include "File.h"
#include <stdexcept>
#include <fstream>

namespace File
{
    std::vector<char> ReadAllBytes(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!"); //Todo: Note file name/path in error. Maybe better to just return an optional
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    std::string ReadToString(const string& path)
    {
        //Read file to string all at once
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in)
            throw std::exception(("Error! Failed to read file \"" + path + "\" into string! Stopping parse.").c_str());

        return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
}