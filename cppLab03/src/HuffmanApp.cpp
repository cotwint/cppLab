#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <queue>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <limits>

// ============================================================================
//  哈夫曼编码综合示例程序
//  说明：本文件整合了 HuffmanNode、HuffmanTree、FileHandler、Application 等类，
//        并提供一个可交互的主程序，演示哈夫曼编码的构建、编码、解码及统计流程。
//        代码包含大量中文注释，帮助理解各个步骤的作用和实现方式。
// ============================================================================

// ============================================================================
//  HuffmanNode 类：用于表示哈夫曼树的节点结构
// ============================================================================
class HuffmanNode {
public:
    char data;                  // 当前节点保存的字符，内部节点使用 '\\0' 作为占位符
    int weight;                 // 节点的权值（出现频率）
    HuffmanNode* left;          // 左子节点指针
    HuffmanNode* right;         // 右子节点指针

    // 构造函数：允许一次性指定字符、权值以及左右子节点
    HuffmanNode(char value, int frequency, HuffmanNode* leftChild = nullptr, HuffmanNode* rightChild = nullptr)
        : data(value), weight(frequency), left(leftChild), right(rightChild) {}
};

// ============================================================================
//  HuffmanTree 类：负责构建哈夫曼树、生成编码表、执行编码解码操作
// ============================================================================
class HuffmanTree {
public:
    HuffmanTree() : root(nullptr) {}

    ~HuffmanTree() {
        destroy(root);
    }

    // 根据字符频率表构建哈夫曼树
    void buildTree(const std::map<char, int>& frequencies) {
        destroy(root);
        root = nullptr;
        codes.clear();

        using NodeQueue = std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeCompare>;
        NodeQueue queue(NodeCompare{});

        // 为每个字符创建叶子节点并压入最小堆
        for (const auto& entry : frequencies) {
            if (entry.second <= 0) {
                continue;
            }
            queue.push(new HuffmanNode(entry.first, entry.second));
        }

        if (queue.empty()) {
            throw std::invalid_argument("Frequency table is empty.");
        }

        // 循环合并最小权值的两个节点直到只剩一个根节点
        while (queue.size() > 1) {
            HuffmanNode* leftChild = queue.top();
            queue.pop();
            HuffmanNode* rightChild = queue.top();
            queue.pop();

            queue.push(new HuffmanNode('\0', leftChild->weight + rightChild->weight, leftChild, rightChild));
        }

        root = queue.top();

        if (isLeaf(root)) {
            codes[root->data] = "0";
        } else {
            buildCodes(root, "");
        }
    }

    // 依据输入文本统计频率并生成编码表
    std::map<char, std::string> encode(const std::string& text) {
        if (text.empty()) {
            throw std::invalid_argument("Input text is empty.");
        }

        std::map<char, int> frequencies;
        for (char ch : text) {
            ++frequencies[ch];
        }

        buildTree(frequencies);

        std::map<char, std::string> orderedCodes;
        for (const auto& entry : codes) {
            orderedCodes.emplace(entry.first, entry.second);
        }
        return orderedCodes;
    }

    // 利用外部编码表将原始文本转为二进制字符串
    std::string encodeText(const std::string& originalText, const std::map<char, std::string>& externalCodes) const {
        if (originalText.empty()) {
            return std::string();
        }

        std::string binary;
        binary.reserve(originalText.size());

        for (char ch : originalText) {
            const auto it = externalCodes.find(ch);
            if (it == externalCodes.end()) {
                throw std::invalid_argument("Character not present in provided Huffman codes.");
            }
            binary += it->second;
        }
        return binary;
    }

    // 将二进制串按照当前树结构解码为原始文本
    std::string decodeText(const std::string& encodedBinary) const {
        if (!root) {
            throw std::runtime_error("Huffman tree is not built.");
        }

        if (encodedBinary.empty()) {
            return std::string();
        }

        std::string decoded;

        if (isLeaf(root)) {
            decoded.reserve(encodedBinary.size());
            for (char bit : encodedBinary) {
                if (bit != '0') {
                    throw std::invalid_argument("Invalid bit for single-node Huffman tree.");
                }
                decoded.push_back(root->data);
            }
            return decoded;
        }

        const HuffmanNode* current = root;
        for (char bit : encodedBinary) {
            if (bit == '0') {
                current = current->left;
            } else if (bit == '1') {
                current = current->right;
            } else {
                throw std::invalid_argument("Encoded text contains invalid characters.");
            }

            if (!current) {
                throw std::invalid_argument("Encoded text does not match Huffman tree.");
            }

            if (isLeaf(current)) {
                decoded.push_back(current->data);
                current = root;
            }
        }

        if (current != root) {
            throw std::invalid_argument("Encoded text ended mid-traversal.");
        }

        return decoded;
    }

    // 打印编码表，便于调试与展示
    void displayCodes(std::ostream& os = std::cout) const {
        if (!root) {
            throw std::runtime_error("Huffman tree is not built.");
        }

        std::vector<std::pair<char, std::string>> orderedCodes(codes.begin(), codes.end());
        std::sort(orderedCodes.begin(), orderedCodes.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.first < rhs.first;
        });

        for (const auto& entry : orderedCodes) {
            if (entry.first == '\0') {
                os << "\\0";
            } else {
                os << entry.first;
            }
            os << ": " << entry.second << '\n';
        }
    }

private:
    struct NodeCompare {
        bool operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const {
            return lhs->weight > rhs->weight;
        }
    };

    HuffmanNode* root;                          // 哈夫曼树根节点
    std::unordered_map<char, std::string> codes; // 内部缓存的编码表

    static bool isLeaf(const HuffmanNode* node) {
        return node && !node->left && !node->right;
    }

    void destroy(HuffmanNode* node) {
        if (!node) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    void buildCodes(HuffmanNode* node, const std::string& prefix) {
        if (!node) {
            return;
        }

        if (isLeaf(node)) {
            codes[node->data] = prefix.empty() ? "0" : prefix;
            return;
        }

        buildCodes(node->left, prefix + '0');
        buildCodes(node->right, prefix + '1');
    }
};

// ============================================================================
//  FileHandler 类：封装文件读写操作，提供文本/二进制读写以及文件大小查询
// ============================================================================
class FileHandler {
public:
    static std::string readTextFile(const std::string& path) {
        std::ifstream input(path, std::ios::in | std::ios::binary);
        if (!input) {
            throw std::runtime_error("无法打开文本文件: " + path);
        }
        return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    }

    static void writeTextFile(const std::string& path, const std::string& content) {
        std::ofstream output(path, std::ios::out | std::ios::binary);
        if (!output) {
            throw std::runtime_error("无法写入文本文件: " + path);
        }
        output.write(content.data(), static_cast<std::streamsize>(content.size()));
    }

    static std::vector<char> readBinaryFile(const std::string& path) {
        std::ifstream input(path, std::ios::in | std::ios::binary);
        if (!input) {
            throw std::runtime_error("无法打开二进制文件: " + path);
        }
        return std::vector<char>((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    }

    static void writeBinaryFile(const std::string& path, const std::vector<char>& binaryData) {
        std::ofstream output(path, std::ios::out | std::ios::binary);
        if (!output) {
            throw std::runtime_error("无法写入二进制文件: " + path);
        }
        output.write(binaryData.data(), static_cast<std::streamsize>(binaryData.size()));
    }

    static std::size_t getFileSize(const std::string& path) {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) {
            throw std::runtime_error("无法获取文件大小: " + path);
        }
        return static_cast<std::size_t>(input.tellg());
    }
};

// ============================================================================
//  Application 类：协调 HuffmanTree 与 FileHandler，用于处理文本分析与文件压缩
// ============================================================================
class Application {
public:
    Application() : originalCharCount(0), encodedBitCount(0) {}

    // 分析文本获取字符频率，为构建哈夫曼树做准备
    void analyzeText(const std::string& text) {
        frequencyTable.clear();
        originalCharCount = text.size();
        for (char ch : text) {
            ++frequencyTable[ch];
        }
    }

    // 压缩指定文件，将频率表和编码结果写入输出文件
    void compressFile(const std::string& inputPath, const std::string& outputPath) {
        const std::string originalText = FileHandler::readTextFile(inputPath);
        analyzeText(originalText);
        if (frequencyTable.empty()) {
            throw std::invalid_argument("Input file is empty, nothing to compress.");
        }

        const auto codes = huffmanTree.encode(originalText);
        const std::string encodedBinary = huffmanTree.encodeText(originalText, codes);
        encodedBitCount = encodedBinary.size();

        std::ofstream output(outputPath, std::ios::out | std::ios::binary);
        if (!output) {
            throw std::runtime_error("Failed to open output file for compression.");
        }

        output << frequencyTable.size() << '\n';
        for (const auto& entry : frequencyTable) {
            output << static_cast<int>(static_cast<unsigned char>(entry.first)) << ' ' << entry.second << '\n';
        }
        output << '\n';
        output << encodedBinary;
        output.close();
    }

    // 解压指定文件，读取频率表并还原原始文本
    void decompressFile(const std::string& inputPath, const std::string& outputPath) {
        std::ifstream input(inputPath, std::ios::in | std::ios::binary);
        if (!input) {
            throw std::runtime_error("Failed to open input file for decompression.");
        }

        std::size_t uniqueCount = 0;
        if (!(input >> uniqueCount)) {
            throw std::runtime_error("Compressed file format error: missing frequency table size.");
        }

        std::map<char, int> loadedFrequencies;
        for (std::size_t i = 0; i < uniqueCount; ++i) {
            int charCode = 0;
            int freq = 0;
            if (!(input >> charCode >> freq)) {
                throw std::runtime_error("Compressed file format error: incomplete frequency entry.");
            }
            loadedFrequencies[static_cast<char>(charCode)] = freq;
        }

        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string encodedBinary;
        std::getline(input, encodedBinary);
        if (encodedBinary.empty()) {
            std::getline(input, encodedBinary);
        }
        input.close();

        frequencyTable = loadedFrequencies;
        originalCharCount = 0;
        encodedBitCount = encodedBinary.size();
        huffmanTree.buildTree(frequencyTable);

        const std::string decodedText = huffmanTree.decodeText(encodedBinary);
        originalCharCount = decodedText.size();

        FileHandler::writeTextFile(outputPath, decodedText);
    }

    // 展示统计信息，包含字符频率和压缩率
    void displayStatistics() const {
        std::cout << "字符频率统计:" << '\n';
        for (const auto& entry : frequencyTable) {
            if (entry.first == '\0') {
                std::cout << "\\0";
            } else {
                std::cout << entry.first;
            }
            std::cout << " -> " << entry.second << '\n';
        }

        std::cout << "原始字符数: " << originalCharCount << '\n';
        std::cout << "编码后比特数: " << encodedBitCount << '\n';
        if (originalCharCount > 0) {
            const double originalBits = static_cast<double>(originalCharCount) * 8.0;
            const double ratio = encodedBitCount / originalBits;
            std::cout << "压缩率(编码/原始): " << ratio << '\n';
        }
    }

    HuffmanTree& getTree() {
        return huffmanTree;
    }

    const std::map<char, int>& getFrequencyTable() const {
        return frequencyTable;
    }

private:
    HuffmanTree huffmanTree;        // 内部持有的哈夫曼树实例
    std::map<char, int> frequencyTable; // 字符频率表
    std::size_t originalCharCount;  // 原始文本总字符数
    std::size_t encodedBitCount;    // 编码后总比特数
};

// ============================================================================
//  辅助结构：用于在菜单模式下缓存当前的编码结果，避免重复计算
// ============================================================================
struct SessionData {
    HuffmanTree tree;
    std::map<char, std::string> codes;
    std::string originalText;
    std::string encodedBinary;
    bool ready = false;
};

// ============================================================================
//  功能函数：执行预置测试用例，验证哈夫曼编码的正确性
// ============================================================================
void runPredefinedTests() {
    const std::vector<std::pair<std::string, std::string>> testCases = {
        {"测试用例 1", "hello world"},
        {"测试用例 2", "the quick brown fox jumps over the lazy dog"},
        {"测试用例 3", "aaaaaaaaabbbbbbcccdde"}
    };

    std::cout << "===== 预置测试用例 =====" << '\n';
    for (const auto& [name, text] : testCases) {
        HuffmanTree tree;
        const auto codes = tree.encode(text);
        const std::string encodedBinary = tree.encodeText(text, codes);
        const std::string decodedText = tree.decodeText(encodedBinary);

        std::cout << name << '\n';
        std::cout << "原始文本: " << text << '\n';
        std::cout << "编码后二进制长度: " << encodedBinary.size() << '\n';
        std::cout << "解码结果: " << decodedText << '\n';
        std::cout << "编码对照表:" << '\n';
        for (const auto& entry : codes) {
            std::cout << "  '";
            if (entry.first == '\0') {
                std::cout << "\\0";
            } else {
                std::cout << entry.first;
            }
            std::cout << "' -> " << entry.second << '\n';
        }
        std::cout << "-----------------------" << '\n';
    }
    std::cout << "===== 测试完成 =====" << '\n' << '\n';
}

// ============================================================================
//  菜单展示函数：打印主菜单供用户选择
// ============================================================================
void showMenu() {
    std::cout << "===== 哈夫曼压缩演示程序 =====" << '\n';
    std::cout << "1. 输入文本进行编码" << '\n';
    std::cout << "2. 使用最新编码结果解码验证" << '\n';
    std::cout << "3. 显示压缩统计信息" << '\n';
    std::cout << "4. 文件压缩 / 解压" << '\n';
    std::cout << "5. 退出程序" << '\n';
    std::cout << "请选择功能编号: ";
}

// ============================================================================
//  处理文本编码：读取用户输入，生成编码并显示结果
// ============================================================================
void handleEncodeText(SessionData& session) {
    std::cout << "请输入要编码的文本: ";
    std::string text;
    std::getline(std::cin, text);

    if (text.empty()) {
        std::cout << "文本为空，无法进行编码。" << '\n';
        return;
    }

    session.originalText = text;
    session.codes = session.tree.encode(text);
    session.encodedBinary = session.tree.encodeText(text, session.codes);
    session.ready = true;

    std::cout << "编码完成。" << '\n';
    std::cout << "原始字符数: " << text.size() << '\n';
    std::cout << "编码后比特数: " << session.encodedBinary.size() << '\n';
    const double originalBits = static_cast<double>(text.size()) * 8.0;
    const double ratio = session.encodedBinary.size() / originalBits;
    std::cout << "压缩率(编码/原始): " << ratio << '\n';

    std::cout << "编码对照表:" << '\n';
    for (const auto& entry : session.codes) {
        std::cout << "  '";
        if (entry.first == '\0') {
            std::cout << "\\0";
        } else {
            std::cout << entry.first;
        }
        std::cout << "' -> " << entry.second << '\n';
    }
}

// ============================================================================
//  处理文本解码：基于最新编码结果执行解码并验证正确性
// ============================================================================
void handleDecodeText(SessionData& session) {
    if (!session.ready) {
        std::cout << "尚未进行编码，请先选择功能 1。" << '\n';
        return;
    }

    const std::string decodedText = session.tree.decodeText(session.encodedBinary);
    std::cout << "解码结果: " << decodedText << '\n';
    if (decodedText == session.originalText) {
        std::cout << "验证成功：解码结果与原文本一致。" << '\n';
    } else {
        std::cout << "警告：解码结果与原文本不一致。" << '\n';
    }
}

// ============================================================================
//  展示压缩统计信息：显示字符数、编码长度以及压缩率
// ============================================================================
void handleStatistics(const SessionData& session) {
    if (!session.ready) {
        std::cout << "暂无统计数据，请先完成一次编码。" << '\n';
        return;
    }

    std::cout << "===== 压缩统计 =====" << '\n';
    std::cout << "原始字符数: " << session.originalText.size() << '\n';
    std::cout << "原始比特数(按8位计): " << session.originalText.size() * 8 << '\n';
    std::cout << "编码后比特数: " << session.encodedBinary.size() << '\n';
    const double originalBits = static_cast<double>(session.originalText.size()) * 8.0;
    const double ratio = session.encodedBinary.size() / originalBits;
    std::cout << "压缩率(编码/原始): " << ratio << '\n';
    std::cout << "====================" << '\n';
}

// ============================================================================
//  文件压缩/解压操作：与用户交互获取文件路径并调用 Application 完成
// ============================================================================
void handleFileOperations(Application& app) {
    std::cout << "请选择文件操作: 1-压缩 2-解压 0-返回菜单: ";
    int choice = 0;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "输入非法，返回主菜单。" << '\n';
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 0) {
        return;
    }

    std::cout << "请输入输入文件路径: ";
    std::string inputPath;
    std::getline(std::cin, inputPath);
    std::cout << "请输入输出文件路径: ";
    std::string outputPath;
    std::getline(std::cin, outputPath);

    try {
        if (choice == 1) {
            app.compressFile(inputPath, outputPath);
            std::size_t originalSize = FileHandler::getFileSize(inputPath);
            std::size_t compressedSize = FileHandler::getFileSize(outputPath);
            std::cout << "压缩完成。原文件大小: " << originalSize
                      << " 字节，压缩后文件大小: " << compressedSize << " 字节" << '\n';
        } else if (choice == 2) {
            app.decompressFile(inputPath, outputPath);
            std::cout << "解压完成。" << '\n';
        } else {
            std::cout << "未知操作，返回主菜单。" << '\n';
        }
    } catch (const std::exception& ex) {
        std::cout << "操作失败: " << ex.what() << '\n';
    }
}

// ============================================================================
//  主函数：程序入口
// ============================================================================
int main() {
    runPredefinedTests();

    Application app;
    SessionData session;

    while (true) {
        showMenu();

        std::string choiceLine;
        std::getline(std::cin, choiceLine);
        if (choiceLine.empty()) {
            std::cout << "未输入选项，请重新选择。" << '\n';
            continue;
        }

        const int choice = choiceLine.front() - '0';
        switch (choice) {
        case 1:
            handleEncodeText(session);
            break;
        case 2:
            handleDecodeText(session);
            break;
        case 3:
            handleStatistics(session);
            break;
        case 4:
            handleFileOperations(app);
            break;
        case 5:
            std::cout << "感谢使用，再见！" << '\n';
            return 0;
        default:
            std::cout << "无效的菜单选项，请重新输入。" << '\n';
            break;
        }

        std::cout << '\n';
    }
}
