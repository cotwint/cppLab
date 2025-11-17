#ifdef _WIN32
#include "../include/ChessBoardWidget.h"
#include <cmath>

ChessBoardWidget::ChessBoardWidget(HWND parent, int x, int y, int width, int height, int boardSize)
    : m_parentHwnd(parent)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_boardSize(boardSize)
{
    m_board.resize(boardSize, -1);
}

ChessBoardWidget::~ChessBoardWidget() {
}

void ChessBoardWidget::setBoard(const std::vector<int>& board) {
    if (static_cast<int>(board.size()) == m_boardSize) {
        m_board = board;
    }
}

void ChessBoardWidget::clear() {
    std::fill(m_board.begin(), m_board.end(), -1);
}

void ChessBoardWidget::paint(HDC hdc) {
    drawBoard(hdc);
}

void ChessBoardWidget::setPosition(int x, int y, int width, int height) {
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

void ChessBoardWidget::drawBoard(HDC hdc) {
    int cellSize = m_width / m_boardSize;

    // 绘制棋盘背景
    HBRUSH whiteBrush = CreateSolidBrush(RGB(240, 240, 240));
    HBRUSH blackBrush = CreateSolidBrush(RGB(100, 150, 100));
    
    for (int row = 0; row < m_boardSize; ++row) {
        for (int col = 0; col < m_boardSize; ++col) {
            int x = m_x + col * cellSize;
            int y = m_y + row * cellSize;
            
            RECT rect = {x, y, x + cellSize, y + cellSize};
            
            // 交替颜色
            if ((row + col) % 2 == 0) {
                FillRect(hdc, &rect, whiteBrush);
            } else {
                FillRect(hdc, &rect, blackBrush);
            }
            
            // 绘制边框
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
            HPEN oldPen = (HPEN)SelectObject(hdc, pen);
            MoveToEx(hdc, x, y, nullptr);
            LineTo(hdc, x + cellSize, y);
            LineTo(hdc, x + cellSize, y + cellSize);
            LineTo(hdc, x, y + cellSize);
            LineTo(hdc, x, y);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);
        }
    }
    
    DeleteObject(whiteBrush);
    DeleteObject(blackBrush);

    // 绘制皇后
    for (int row = 0; row < m_boardSize; ++row) {
        if (m_board[row] >= 0 && m_board[row] < m_boardSize) {
            drawQueen(hdc, row, m_board[row], cellSize);
        }
    }

    // 绘制行列标注
    HFONT hFont = CreateFontW(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Arial"
    );
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    for (int i = 0; i < m_boardSize; ++i) {
        wchar_t text[3];
        
        // 行号
        wsprintfW(text, L"%d", i + 1);
        RECT textRect = {m_x - 20, m_y + i * cellSize + cellSize / 2 - 8, m_x, m_y + i * cellSize + cellSize / 2 + 8};
        DrawTextW(hdc, text, -1, &textRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        
        // 列号（字母）
        text[0] = L'A' + i;
        text[1] = L'\0';
        textRect = {m_x + i * cellSize, m_y + m_height + 5, m_x + (i + 1) * cellSize, m_y + m_height + 25};
        DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

void ChessBoardWidget::drawQueen(HDC hdc, int row, int col, int cellSize) {
    int x = m_x + col * cellSize + cellSize / 2;
    int y = m_y + row * cellSize + cellSize / 2;
    int radius = cellSize / 3;

    // 绘制皇后主体（圆形）
    HBRUSH queenBrush = CreateSolidBrush(RGB(220, 20, 60));
    HPEN queenPen = CreatePen(PS_SOLID, 2, RGB(139, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, queenPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, queenBrush);

    Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);

    // 绘制皇冠
    POINT crown[7];
    int crownHeight = radius / 2;
    crown[0] = {x - radius, y - radius};
    crown[1] = {x - radius * 2 / 3, y - radius - crownHeight};
    crown[2] = {x - radius / 3, y - radius};
    crown[3] = {x, y - radius - crownHeight};
    crown[4] = {x + radius / 3, y - radius};
    crown[5] = {x + radius * 2 / 3, y - radius - crownHeight};
    crown[6] = {x + radius, y - radius};

    Polygon(hdc, crown, 7);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(queenBrush);
    DeleteObject(queenPen);

    // 绘制标记（Q字母）
    HFONT hFont = CreateFontW(
        radius, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Arial"
    );
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    RECT textRect = {x - radius, y - radius / 2, x + radius, y + radius / 2};
    DrawTextW(hdc, L"♛", -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

#endif // _WIN32
