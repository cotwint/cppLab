#pragma once

#include <vector>

#include "Solution.h"
#include "Stack.h"

class Queen {
private:
    std::vector<int> m_board;
    std::vector<std::vector<int>> m_solutions;
    int m_boardSize;
    std::vector<bool> m_colUsed;
    std::vector<bool> m_diag1Used;
    std::vector<bool> m_diag2Used;
    Stack m_stack;

    void resetState();
    bool isValid(int row, int col) const;
    bool isColumnSafe(int col) const;
    bool isDiagonal1Safe(int row, int col) const;
    bool isDiagonal2Safe(int row, int col) const;
    void placeQueen(int row, int col);
    void removeQueen(int row, int col);
    void saveSolution();
    void solveRecursiveHelper(int row);
    void solveIterativeHelper();

public:
    explicit Queen(int size = 8);
    ~Queen();

    void solveRecursive();
    void solveIterative();

    void displayAllSolutions();
    void displaySolution(int index);
    int getSolutionCount() const;
    std::vector<Solution> getSolutions() const;
};
