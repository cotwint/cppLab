#pragma once

#ifdef _WIN32
#include <windows.h>
#include <string>
#include <memory>
#include "Queen.h"

class ChessBoardWidget;

class MainWindow {
private:
    HWND m_hwnd;
    HWND m_btnSolveRecursive;
    HWND m_btnSolveIterative;
    HWND m_btnPrevSolution;
    HWND m_btnNextSolution;
    HWND m_btnReset;
    HWND m_lblSolutionInfo;
    HWND m_lblStatus;
    std::unique_ptr<ChessBoardWidget> m_chessBoard;
    std::unique_ptr<Queen> m_queen;
    int m_currentSolutionIndex;
    int m_boardSize;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onCreate();
    void onCommand(WPARAM wParam);
    void onPaint();
    void onSize(int width, int height);
    void updateDisplay();
    void updateStatusLabel();
    void solveRecursive();
    void solveIterative();
    void showPreviousSolution();
    void showNextSolution();
    void reset();

public:
    MainWindow();
    ~MainWindow();

    bool create(HINSTANCE hInstance, int nCmdShow);
    HWND getHandle() const { return m_hwnd; }
};

#endif // _WIN32
