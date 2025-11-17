#pragma once

#ifdef _WIN32
#include <windows.h>
#include <vector>

class ChessBoardWidget {
private:
    HWND m_parentHwnd;
    int m_x, m_y, m_width, m_height;
    std::vector<int> m_board;
    int m_boardSize;
    
    void drawBoard(HDC hdc);
    void drawQueen(HDC hdc, int row, int col, int cellSize);

public:
    ChessBoardWidget(HWND parent, int x, int y, int width, int height, int boardSize = 8);
    ~ChessBoardWidget();

    void setBoard(const std::vector<int>& board);
    void clear();
    void paint(HDC hdc);
    void setPosition(int x, int y, int width, int height);
};

#endif // _WIN32
