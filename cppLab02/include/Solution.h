#pragma once

#include <vector>
#include <iosfwd>

class Solution {
private:
    std::vector<int> m_positions;
    int m_solutionID{};

public:
    Solution(const std::vector<int>& positions, int id);

    void display() const;
    bool verify() const;
    std::vector<int> getPositions() const;
    int getId() const;
};

void displayBoard(const std::vector<int>& board);
