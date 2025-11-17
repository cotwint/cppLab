#include "../include/HuffmanTree.h"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <map>

HuffmanNode::HuffmanNode(char value, int frequency, HuffmanNode* leftChild, HuffmanNode* rightChild)
    : data(value), weight(frequency), left(leftChild), right(rightChild) {}

HuffmanTree::HuffmanTree() : root(nullptr) {}

HuffmanTree::~HuffmanTree() {
    destroy(root);
}

bool HuffmanTree::NodeCompare::operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const {
    return lhs->weight > rhs->weight;
}

void HuffmanTree::buildTree(const std::map<char, int>& frequencies) {
    // 先释放原有树结构，确保重新构建时不会出现内存泄漏
    destroy(root);
    root = nullptr;
    codes.clear();

    using NodeQueue = std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeCompare>;
    NodeQueue queue(NodeCompare{});

    // 遍历频率表，为每个字符创建一个叶子节点并压入最小堆
    for (const auto& entry : frequencies) {
        // 约定权值小于等于 0 的字符忽略，避免无效节点影响构建
        if (entry.second <= 0) {
            continue;
        }
        queue.push(new HuffmanNode(entry.first, entry.second));
    }

    // 若最小堆为空，说明输入为空，无法构建哈夫曼树
    if (queue.empty()) {
        throw std::invalid_argument("Frequency table is empty.");
    }

    // 重复取出权值最小的两个节点，合并成新的父节点，再放回最小堆
    while (queue.size() > 1) {
        // 取出权值最小的左子节点
        HuffmanNode* leftChild = queue.top();
        queue.pop();
        // 取出权值次小的右子节点
        HuffmanNode* rightChild = queue.top();
        queue.pop();

        // 新父节点的字符设为 '\0' 占位，权值为两个子节点之和
        queue.push(new HuffmanNode('\0', leftChild->weight + rightChild->weight, leftChild, rightChild));
    }

    // 最后剩下的节点即为哈夫曼树的根节点
    root = queue.top();

    // 若树只有单个节点，需要特殊处理编码，确保至少生成一位
    if (isLeaf(root)) {
        codes[root->data] = "0";
    } else {
        buildCodes(root, "");
    }
}

std::map<char, std::string> HuffmanTree::encode(const std::string& text) {
    // 输入字符串为空时无法统计频率，直接抛出异常提示调用者
    if (text.empty()) {
        throw std::invalid_argument("Input text is empty.");
    }

    // 统计每个字符出现的频率，形成构建哈夫曼树所需的权值表
    std::map<char, int> frequencies;
    for (char ch : text) {
        ++frequencies[ch];
    }

    // 调用 buildTree 根据频率表构建哈夫曼树，同时生成内部的编码缓存
    buildTree(frequencies);

    // 将内部使用的 unordered_map 拷贝到 map 中，满足有序输出的需求
    std::map<char, std::string> orderedCodes;
    for (const auto& entry : codes) {
        orderedCodes.emplace(entry.first, entry.second);
    }

    return orderedCodes;
}

std::string HuffmanTree::encodeText(const std::string& originalText, const std::map<char, std::string>& externalCodes) const {
    // 若原始文本为空，直接返回空字符串，表示无需编码
    if (originalText.empty()) {
        return std::string();
    }

    std::string binary;
    binary.reserve(originalText.size());

    // 遍历原始文本中的每个字符，查找其对应的哈夫曼编码
    for (char ch : originalText) {
        const auto it = externalCodes.find(ch);
        if (it == externalCodes.end()) {
            // 若字符未出现在编码表中，说明树与输入不匹配，抛出异常提示调用者
            throw std::invalid_argument("Character not present in provided Huffman codes.");
        }
        // 将查到的编码追加到结果字符串中
        binary += it->second;
    }

    return binary;
}

std::string HuffmanTree::decodeText(const std::string& encodedBinary) const {
    // 若哈夫曼树尚未构建，则无法进行解码
    if (!root) {
        throw std::runtime_error("Huffman tree is not built.");
    }

    // 输入为空时直接返回空字符串，表示无任何数据需要解码
    if (encodedBinary.empty()) {
        return std::string();
    }

    std::string decoded;

    // 单节点树（只包含一个字符）需要特殊处理：任何有效编码只能由 '0' 组成
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

    // 从根节点开始逐位遍历编码串，遇到叶子节点即得到一个原字符
    const HuffmanNode* current = root;
    for (char bit : encodedBinary) {
        if (bit == '0') {
            current = current->left;
        } else if (bit == '1') {
            current = current->right;
        } else {
            // 出现除 '0'、'1' 外的字符，说明编码串非法
            throw std::invalid_argument("Encoded text contains invalid characters.");
        }

        if (!current) {
            // 走到空指针说明编码串与当前树结构不匹配
            throw std::invalid_argument("Encoded text does not match Huffman tree.");
        }

        if (isLeaf(current)) {
            // 叶子节点代表一个完整字符，写入结果，并重置到根节点继续解析剩余比特
            decoded.push_back(current->data);
            current = root;
        }
    }

    // 遍历结束后若未回到根节点，说明编码串截止在一条路径中部，属于非法输入
    if (current != root) {
        throw std::invalid_argument("Encoded text ended mid-traversal.");
    }

    return decoded;
}

std::string HuffmanTree::decode(const std::string& encodedText) const {
    return decodeText(encodedText);
}

void HuffmanTree::displayCodes(std::ostream& os) const {
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

void HuffmanTree::destroy(HuffmanNode* node) {
    if (!node) {
        return;
    }

    destroy(node->left);
    destroy(node->right);
    delete node;
}

void HuffmanTree::buildCodes(HuffmanNode* node, const std::string& prefix) {
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

bool HuffmanTree::isLeaf(const HuffmanNode* node) {
    return node && !node->left && !node->right;
}
