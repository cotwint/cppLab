#include "../include/Application.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include "../include/FileHandler.h"

Application::Application()
    : originalCharCount(0), encodedBitCount(0) {}

void Application::analyzeText(const std::string& text) {
    // 清空旧的统计结果，为新文本重新统计字符频率
    frequencyTable.clear();
    originalCharCount = text.size();

    // 遍历文本中的每个字符，累加出现次数
    for (char ch : text) {
        ++frequencyTable[ch];
    }
}

void Application::compressFile(const std::string& inputPath, const std::string& outputPath) {
    // 读取输入文件的完整内容
    const std::string originalText = FileHandler::readTextFile(inputPath);

    // 对文本进行频率分析并构建哈夫曼树
    analyzeText(originalText);
    if (frequencyTable.empty()) {
        throw std::invalid_argument("Input file is empty, nothing to compress.");
    }

    // 使用哈夫曼树生成编码表
    const auto codes = huffmanTree.encode(originalText);

    // 将原始文本转换为二进制编码串
    const std::string encodedBinary = huffmanTree.encodeText(originalText, codes);
    encodedBitCount = encodedBinary.size();

    // 将频率表和编码结果写入输出文件，便于后续解压还原
    std::ofstream output(outputPath, std::ios::out | std::ios::binary);
    if (!output) {
        throw std::runtime_error("Failed to open output file for compression.");
    }

    // 写入字符种类数量，方便解码时读取
    output << frequencyTable.size() << '\n';
    for (const auto& entry : frequencyTable) {
        // 以字符 ASCII 值 + 频率的形式存储，避免因特殊字符造成解析歧义
        output << static_cast<int>(static_cast<unsigned char>(entry.first)) << ' ' << entry.second << '\n';
    }

    // 插入一行空行作为频率表与编码数据的分隔
    output << '\n';
    // 写入实际的二进制编码串
    output << encodedBinary;
    output.close();
}

void Application::decompressFile(const std::string& inputPath, const std::string& outputPath) {
    // 打开压缩文件以读取频率表和编码数据
    std::ifstream input(inputPath, std::ios::in | std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open input file for decompression.");
    }

    std::size_t uniqueCount = 0;
    if (!(input >> uniqueCount)) {
        throw std::runtime_error("Compressed file format error: missing frequency table size.");
    }

    // 逐项读取频率表，恢复为 map<char, int>
    std::map<char, int> loadedFrequencies;
    for (std::size_t i = 0; i < uniqueCount; ++i) {
        int charCode = 0;
        int freq = 0;
        if (!(input >> charCode >> freq)) {
            throw std::runtime_error("Compressed file format error: incomplete frequency entry.");
        }
        loadedFrequencies[static_cast<char>(charCode)] = freq;
    }

    // 跳过分隔行，定位到编码数据开始的位置
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string encodedBinary;
    // 跳过频率表与编码数据之间的空行
    std::getline(input, encodedBinary);
    if (encodedBinary.empty()) {
        std::getline(input, encodedBinary);
    }
    input.close();

    if (encodedBinary.empty()) {
        throw std::runtime_error("Compressed file contains empty encoded data.");
    }

    // 使用读取到的频率表重建哈夫曼树
    frequencyTable = loadedFrequencies;
    originalCharCount = 0;
    encodedBitCount = encodedBinary.size();
    huffmanTree.buildTree(frequencyTable);

    // 利用哈夫曼树将二进制串解码为原始文本
    const std::string decodedText = huffmanTree.decodeText(encodedBinary);
    originalCharCount = decodedText.size();

    // 将解码后的文本写入输出文件
    std::ofstream output(outputPath, std::ios::out | std::ios::binary);
    if (!output) {
        throw std::runtime_error("Failed to open output file for decompression.");
    }
    output.write(decodedText.data(), static_cast<std::streamsize>(decodedText.size()));
    output.close();
}

void Application::displayStatistics() const {
    // 展示当前频率统计信息和压缩效率指标
    std::cout << "Character Frequencies:" << '\n';
    for (const auto& entry : frequencyTable) {
        if (entry.first == '\0') {
            std::cout << "\\0";
        } else {
            std::cout << entry.first;
        }
        std::cout << " -> " << entry.second << '\n';
    }

    std::cout << "Original characters: " << originalCharCount << '\n';
    std::cout << "Encoded bits: " << encodedBitCount << '\n';
    if (originalCharCount > 0) {
        const double originalBits = static_cast<double>(originalCharCount) * 8.0;
        const double ratio = encodedBitCount / originalBits;
        std::cout << "Compression ratio (encoded/original): " << ratio << '\n';
    }
}
