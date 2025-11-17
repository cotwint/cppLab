#pragma once

#include <string>
#include <unordered_map>
#include <queue>
#include <map>
#include <iostream>

class HuffmanNode {
public:
    char data;
    int weight;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char value, int frequency, HuffmanNode* leftChild = nullptr, HuffmanNode* rightChild = nullptr);
};

class HuffmanTree {
public:
    HuffmanTree();
    ~HuffmanTree();

    void buildTree(const std::map<char, int>& frequencies);
    std::map<char, std::string> encode(const std::string& text);
    std::string encodeText(const std::string& originalText, const std::map<char, std::string>& codes) const;
    std::string decodeText(const std::string& encodedBinary) const;
    std::string decode(const std::string& encodedText) const;
    void displayCodes(std::ostream& os = std::cout) const;

private:
    struct NodeCompare {
        bool operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const;
    };

    HuffmanNode* root;
    std::unordered_map<char, std::string> codes;

    void destroy(HuffmanNode* node);
    void buildCodes(HuffmanNode* node, const std::string& prefix);
    static bool isLeaf(const HuffmanNode* node);
};
