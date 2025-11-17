#include "../include/FileHandler.h"

#include <fstream>
#include <stdexcept>
#include <vector>

std::string FileHandler::readTextFile(const std::string& path) {
    // 以二进制模式防止平台差异造成的换行符转换，便于压缩过程中保持字节一致性
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) {
        throw std::runtime_error("无法打开文本文件: " + path);
    }

    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

void FileHandler::writeTextFile(const std::string& path, const std::string& content) {
    // 同样采用二进制模式写入，避免系统自动转换换行符导致的数据偏差
    std::ofstream output(path, std::ios::out | std::ios::binary);
    if (!output) {
        throw std::runtime_error("无法写入文本文件: " + path);
    }

    output.write(content.data(), static_cast<std::streamsize>(content.size()));
}

std::vector<char> FileHandler::readBinaryFile(const std::string& path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) {
        throw std::runtime_error("无法打开二进制文件: " + path);
    }

    return std::vector<char>((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

void FileHandler::writeBinaryFile(const std::string& path, const std::vector<char>& binaryData) {
    std::ofstream output(path, std::ios::out | std::ios::binary);
    if (!output) {
        throw std::runtime_error("无法写入二进制文件: " + path);
    }

    output.write(binaryData.data(), static_cast<std::streamsize>(binaryData.size()));
}

std::size_t FileHandler::getFileSize(const std::string& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        throw std::runtime_error("无法获取文件大小: " + path);
    }

    return static_cast<std::size_t>(input.tellg());
}
