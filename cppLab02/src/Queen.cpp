#include "../include/Queen.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

Queen::Queen(int size)
    : m_board(size, -1),
      m_boardSize(size),
      m_colUsed(size, false),
    m_diag1Used(2 * size - 1, false),
    m_diag2Used(2 * size - 1, false) {}

Queen::~Queen() = default;

void Queen::solveRecursive() {
    m_solutions.clear();
    resetState();
    solveRecursiveHelper(0);
}

void Queen::solveIterative() {
    m_solutions.clear();
    resetState();
    solveIterativeHelper();
}

void Queen::displayAllSolutions() {
    const int count = static_cast<int>(m_solutions.size());
    for (int i = 0; i < count; ++i) {
        Solution sol(m_solutions[i], i + 1);
        sol.display();
    }
}

void Queen::displaySolution(int index) {
    if (index < 0 || index >= static_cast<int>(m_solutions.size())) {
        throw std::out_of_range("solution index out of range");
    }
    Solution sol(m_solutions[index], index + 1);
    sol.display();
}

int Queen::getSolutionCount() const {
    return static_cast<int>(m_solutions.size());
}

std::vector<Solution> Queen::getSolutions() const {
    std::vector<Solution> result;
    result.reserve(m_solutions.size());
    for (std::size_t i = 0; i < m_solutions.size(); ++i) {
        result.emplace_back(m_solutions[i], static_cast<int>(i) + 1);
    }
    return result;
}

void Queen::resetState() {
    std::fill(m_board.begin(), m_board.end(), -1);
    std::fill(m_colUsed.begin(), m_colUsed.end(), false);
    std::fill(m_diag1Used.begin(), m_diag1Used.end(), false);
    std::fill(m_diag2Used.begin(), m_diag2Used.end(), false);
    m_stack.clear();
}

bool Queen::isValid(int row, int col) const {
    return isColumnSafe(col) && isDiagonal1Safe(row, col) && isDiagonal2Safe(row, col);
}

bool Queen::isColumnSafe(int col) const {
    return !m_colUsed[col];
}

bool Queen::isDiagonal1Safe(int row, int col) const {
    return !m_diag1Used[row - col + m_boardSize - 1];
}

bool Queen::isDiagonal2Safe(int row, int col) const {
    return !m_diag2Used[row + col];
}

void Queen::placeQueen(int row, int col) {
    m_board[row] = col;
    m_colUsed[col] = true;
    m_diag1Used[row - col + m_boardSize - 1] = true;
    m_diag2Used[row + col] = true;
}

void Queen::removeQueen(int row, int col) {
    m_board[row] = -1;
    m_colUsed[col] = false;
    m_diag1Used[row - col + m_boardSize - 1] = false;
    m_diag2Used[row + col] = false;
}

void Queen::saveSolution() {
    m_solutions.push_back(m_board);
}

void Queen::solveRecursiveHelper(int row) {
    if (row == m_boardSize) {
        saveSolution();
        return;
    }

    for (int col = 0; col < m_boardSize; ++col) {
        if (!isValid(row, col)) {
            continue;
        }
        placeQueen(row, col);
        solveRecursiveHelper(row + 1);
        removeQueen(row, col);
    }
}

void Queen::solveIterativeHelper() {
    int row = 0;
    int col = 0;

    while (true) {
        while (row < m_boardSize) {
            bool placed = false;
            for (; col < m_boardSize; ++col) {
                if (!isValid(row, col)) {
                    continue;
                }
                placeQueen(row, col);
                m_stack.push({row, col, col + 1});
                ++row;
                col = 0;
                placed = true;
                break;
            }

            if (!placed) {
                if (m_stack.isEmpty()) {
                    return;
                }
                State backtrack = m_stack.pop();
                removeQueen(backtrack.row, backtrack.col);
                row = backtrack.row;
                col = backtrack.nextCol;
            }
        }

        saveSolution();

        if (m_stack.isEmpty()) {
            return;
        }

        State backtrack = m_stack.pop();
        removeQueen(backtrack.row, backtrack.col);
        row = backtrack.row;
        col = backtrack.nextCol;
    }
}
