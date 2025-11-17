#include "../include/Solution.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {
void printBoardRowHeader(int size) {
    std::wcout << L"  ";
    for (int col = 0; col < size; ++col) {
        std::wcout << col << L' ';
    }
    std::wcout << L'\n';
}
}

void displayBoard(const std::vector<int>& board) {
    const int size = static_cast<int>(board.size());
    printBoardRowHeader(size);
    for (int row = 0; row < size; ++row) {
        std::wcout << row << L' ';
        for (int col = 0; col < size; ++col) {
            const wchar_t cell = (board[row] == col) ? L'Q' : L'.';
            std::wcout << cell << L' ';
        }
        std::wcout << L'\n';
    }
}

Solution::Solution(const std::vector<int>& positions, int id)
    : m_positions(positions), m_solutionID(id) {}

void Solution::display() const {
    std::wcout << L"解 #" << m_solutionID << L":\n";
    displayBoard(m_positions);
    std::wostringstream oss;
    oss << L'[';
    for (std::size_t i = 0; i < m_positions.size(); ++i) {
        oss << m_positions[i];
        if (i + 1 < m_positions.size()) {
            oss << L", ";
        }
    }
    oss << L']';
    std::wcout << L"\n皇后位置: " << oss.str() << L'\n';
    std::wcout << L"合法性验证: " << (verify() ? L"通过" : L"失败") << L"\n\n";
}

bool Solution::verify() const {
    const int size = static_cast<int>(m_positions.size());
    if (size == 0) {
        return false;
    }

    std::vector<bool> colUsed(size, false);
    std::vector<bool> diag1(2 * size - 1, false);
    std::vector<bool> diag2(2 * size - 1, false);

    for (int row = 0; row < size; ++row) {
        const int col = m_positions[row];
        if (col < 0 || col >= size) {
            return false;
        }

        if (colUsed[col]) {
            return false;
        }
        colUsed[col] = true;

        const int idxDiag1 = row - col + size - 1;
        const int idxDiag2 = row + col;

        if (diag1[idxDiag1] || diag2[idxDiag2]) {
            return false;
        }
        diag1[idxDiag1] = true;
        diag2[idxDiag2] = true;
    }

    return true;
}

std::vector<int> Solution::getPositions() const {
    return m_positions;
}

int Solution::getId() const {
    return m_solutionID;
}
