#include <algorithm>
#include <cassert>
#include <clocale>
#include <exception>
#include <iostream>
#include <limits>
#include <locale>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include "../include/Queen.h"
#include "../include/Solution.h"

namespace {

int getValidInput(int min, int max) {
    int value{};
    while (true) {
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::wcout << L"输入无效，请输入" << min << L"到" << max << L"之间的整数: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void displayMenu() {
    std::wcout << L"\n┌─────────────────────────────────┐\n"
                 L"│   八皇后问题求解系统 v1.0      │\n"
                 L"├─────────────────────────────────┤\n"
                 L"│  1. 使用递归算法求解           │\n"
                 L"│  2. 使用非递归算法求解         │\n"
                 L"│  3. 显示第N个解                 │\n"
                 L"│  4. 显示所有解                  │\n"
                 L"│  5. 查看解的总数                │\n"
                 L"│  6. 重置程序                    │\n"
                 L"│  7. 退出程序                    │\n"
                 L"├─────────────────────────────────┤\n"
                 L"│  请选择操作 (1-7):              │\n"
                 L"└─────────────────────────────────┘\n";
}

void handleUserChoice(int choice, Queen& queen, int boardSize) {
    try {
        switch (choice) {
        case 1:
            queen.solveRecursive();
            std::wcout << L"递归算法求解完成，共找到" << queen.getSolutionCount() << L"个解。\n";
            break;
        case 2:
            queen.solveIterative();
            std::wcout << L"非递归算法求解完成，共找到" << queen.getSolutionCount() << L"个解。\n";
            break;
        case 3: {
            const int count = queen.getSolutionCount();
            if (count == 0) {
                std::wcout << L"请先选择求解算法以生成解。\n";
                break;
            }
            std::wcout << L"请输入要显示的解序号 (1-" << count << L"): ";
            const int index = getValidInput(1, count);
            queen.displaySolution(index - 1);
            break;
        }
        case 4:
            if (queen.getSolutionCount() == 0) {
                std::wcout << L"请先选择求解算法以生成解。\n";
                break;
            }
            queen.displayAllSolutions();
            break;
        case 5:
            std::wcout << L"当前解的总数为: " << queen.getSolutionCount() << L'\n';
            break;
        case 6:
            queen = Queen(boardSize);
            std::wcout << L"程序已重置。\n";
            break;
        default:
            std::wcout << L"未知选项，请重新选择。\n";
            break;
        }
    } catch (const std::exception& e) {
        std::wcerr << L"错误: " << e.what() << L"\n";
    }
}

#if !defined(NDEBUG)
void runTests() {
    Queen queen;
    queen.solveRecursive();
    assert(queen.getSolutionCount() == 92);
    const auto solutions = queen.getSolutions();
    if (!solutions.empty()) {
        const std::vector<int> expected{0, 4, 7, 5, 2, 6, 1, 3};
        assert(solutions.front().getPositions() == expected);
    }

    Queen queen2;
    queen2.solveIterative();
    assert(queen2.getSolutionCount() == 92);
    const auto solutionsIter = queen2.getSolutions();
    assert(solutionsIter.size() == solutions.size());
    for (std::size_t i = 0; i < solutions.size() && i < solutionsIter.size(); ++i) {
        assert(solutions[i].verify());
        assert(solutionsIter[i].verify());
        assert(solutions[i].getPositions() == solutionsIter[i].getPositions());
    }
}
#endif

void mainMenu() {
    const int boardSize = 8;
    Queen queen(boardSize);

    bool running = true;
    while (running) {
        displayMenu();
        const int choice = getValidInput(1, 7);
        if (choice == 7) {
            running = false;
            std::wcout << L"感谢使用，再见！\n";
        } else {
            handleUserChoice(choice, queen, boardSize);
        }
    }
}

} // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif
    std::setlocale(LC_ALL, "");
    std::wcout.imbue(std::locale());
    std::wcerr.imbue(std::locale());
#ifndef NDEBUG
    runTests();
#endif
    mainMenu();
    return 0;
}
