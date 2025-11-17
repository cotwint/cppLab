#pragma once

#include "HuffmanTree.h"

#include <string>
#include <map>
#include <cstddef>

class Application {
public:
    Application();

    void analyzeText(const std::string& text);
    void compressFile(const std::string& inputPath, const std::string& outputPath);
    void decompressFile(const std::string& inputPath, const std::string& outputPath);
    void displayStatistics() const;

private:
    HuffmanTree huffmanTree;
    std::map<char, int> frequencyTable;
    std::size_t originalCharCount;
    std::size_t encodedBitCount;
};
