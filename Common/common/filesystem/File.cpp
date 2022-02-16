#include "File.h"
#include <stdexcept>
#include <fstream>

namespace File
{
    std::vector<u8> ReadAllBytes(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!"); //Todo: Note file name/path in error. Maybe better to just return an optional
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<u8> buffer(fileSize);

        file.seekg(0);
        file.read((char*)buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    std::span<u8> ReadAllBytesSpan(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("failed to open file!"); //Todo: Note file name/path in error. Maybe better to just return an optional

        size_t fileSize = static_cast<size_t>(file.tellg());
        u8* buffer = new u8[fileSize];

        file.seekg(0);
        file.read((char*)buffer, fileSize);
        file.close();

        return std::span<u8>(buffer, fileSize);
    }

    std::string ReadToString(const string& path)
    {
        //Read file to string all at once
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in)
            throw std::exception(("Error! Failed to read file \"" + path + "\" into string! Stopping parse.").c_str());

        return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    void WriteToFile(const string& path, std::span<u8> data)
    {
        std::ofstream stream(path, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        stream.write(reinterpret_cast<const char*>(data.data()), data.size_bytes());
    }
}