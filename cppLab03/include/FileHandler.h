#pragma once

#include <string>
#include <vector>

class FileHandler {
public:
    static std::string readTextFile(const std::string& path);
    static void writeTextFile(const std::string& path, const std::string& content);
    static std::vector<char> readBinaryFile(const std::string& path);
    static void writeBinaryFile(const std::string& path, const std::vector<char>& binaryData);
    static std::size_t getFileSize(const std::string& path);
};
