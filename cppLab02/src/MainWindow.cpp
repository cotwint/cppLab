#ifdef _WIN32
#include "../include/MainWindow.h"
#include "../include/ChessBoardWidget.h"
#include <sstream>
#include <iomanip>

#define ID_BTN_SOLVE_RECURSIVE 101
#define ID_BTN_SOLVE_ITERATIVE 102
#define ID_BTN_PREV_SOLUTION 103
#define ID_BTN_NEXT_SOLUTION 104
#define ID_BTN_RESET 105

MainWindow::MainWindow()
    : m_hwnd(nullptr)
    , m_btnSolveRecursive(nullptr)
    , m_btnSolveIterative(nullptr)
    , m_btnPrevSolution(nullptr)
    , m_btnNextSolution(nullptr)
    , m_btnReset(nullptr)
    , m_lblSolutionInfo(nullptr)
    , m_lblStatus(nullptr)
    , m_chessBoard(nullptr)
    , m_queen(nullptr)
    , m_currentSolutionIndex(0)
    , m_boardSize(8)
{
    m_queen = std::make_unique<Queen>(m_boardSize);
}

MainWindow::~MainWindow() {
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hwnd = hwnd;
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
        case WM_CREATE:
            pThis->onCreate();
            return 0;
        case WM_COMMAND:
            pThis->onCommand(wParam);
            return 0;
        case WM_PAINT:
            pThis->onPaint();
            return 0;
        case WM_SIZE:
            pThis->onSize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool MainWindow::create(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"EightQueensWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"八皇后问题求解系统",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 700,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (m_hwnd == nullptr) {
        return false;
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

void MainWindow::onCreate() {
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE));

    // 创建按钮
    m_btnSolveRecursive = CreateWindowW(
        L"BUTTON", L"递归求解",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 520, 120, 35,
        m_hwnd, reinterpret_cast<HMENU>(ID_BTN_SOLVE_RECURSIVE), hInstance, nullptr
    );

    m_btnSolveIterative = CreateWindowW(
        L"BUTTON", L"非递归求解",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        160, 520, 120, 35,
        m_hwnd, reinterpret_cast<HMENU>(ID_BTN_SOLVE_ITERATIVE), hInstance, nullptr
    );

    m_btnPrevSolution = CreateWindowW(
        L"BUTTON", L"← 上一个",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 570, 100, 35,
        m_hwnd, reinterpret_cast<HMENU>(ID_BTN_PREV_SOLUTION), hInstance, nullptr
    );

    m_btnNextSolution = CreateWindowW(
        L"BUTTON", L"下一个 →",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        140, 570, 100, 35,
        m_hwnd, reinterpret_cast<HMENU>(ID_BTN_NEXT_SOLUTION), hInstance, nullptr
    );

    m_btnReset = CreateWindowW(
        L"BUTTON", L"重置",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        300, 520, 100, 35,
        m_hwnd, reinterpret_cast<HMENU>(ID_BTN_RESET), hInstance, nullptr
    );

    // 创建标签
    m_lblSolutionInfo = CreateWindowW(
        L"STATIC", L"未求解",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 620, 400, 25,
        m_hwnd, nullptr, hInstance, nullptr
    );

    m_lblStatus = CreateWindowW(
        L"STATIC", L"状态: 就绪",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 645, 400, 25,
        m_hwnd, nullptr, hInstance, nullptr
    );

    // 创建棋盘显示组件
    m_chessBoard = std::make_unique<ChessBoardWidget>(m_hwnd, 20, 20, 480, 480, m_boardSize);

    // 设置字体
    HFONT hFont = CreateFontW(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Microsoft YaHei"
    );

    SendMessage(m_btnSolveRecursive, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessage(m_btnSolveIterative, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessage(m_btnPrevSolution, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessage(m_btnNextSolution, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessage(m_btnReset, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessage(m_lblSolutionInfo, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessage(m_lblStatus, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    // 初始禁用导航按钮
    EnableWindow(m_btnPrevSolution, FALSE);
    EnableWindow(m_btnNextSolution, FALSE);
}

void MainWindow::onCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
    case ID_BTN_SOLVE_RECURSIVE:
        solveRecursive();
        break;
    case ID_BTN_SOLVE_ITERATIVE:
        solveIterative();
        break;
    case ID_BTN_PREV_SOLUTION:
        showPreviousSolution();
        break;
    case ID_BTN_NEXT_SOLUTION:
        showNextSolution();
        break;
    case ID_BTN_RESET:
        reset();
        break;
    }
}

void MainWindow::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    
    if (m_chessBoard) {
        m_chessBoard->paint(hdc);
    }
    
    EndPaint(m_hwnd, &ps);
}

void MainWindow::onSize(int width, int height) {
    // 可以在这里实现窗口大小改变时的布局调整
}

void MainWindow::solveRecursive() {
    SetWindowTextW(m_lblStatus, L"状态: 正在使用递归算法求解...");
    UpdateWindow(m_hwnd);

    m_queen->solveRecursive();
    m_currentSolutionIndex = 0;
    
    updateDisplay();
    updateStatusLabel();
    
    std::wstringstream ss;
    ss << L"状态: 递归算法求解完成，共找到 " << m_queen->getSolutionCount() << L" 个解";
    SetWindowTextW(m_lblStatus, ss.str().c_str());

    EnableWindow(m_btnPrevSolution, m_queen->getSolutionCount() > 1);
    EnableWindow(m_btnNextSolution, m_queen->getSolutionCount() > 1);
}

void MainWindow::solveIterative() {
    SetWindowTextW(m_lblStatus, L"状态: 正在使用非递归算法求解...");
    UpdateWindow(m_hwnd);

    m_queen->solveIterative();
    m_currentSolutionIndex = 0;
    
    updateDisplay();
    updateStatusLabel();
    
    std::wstringstream ss;
    ss << L"状态: 非递归算法求解完成，共找到 " << m_queen->getSolutionCount() << L" 个解";
    SetWindowTextW(m_lblStatus, ss.str().c_str());

    EnableWindow(m_btnPrevSolution, m_queen->getSolutionCount() > 1);
    EnableWindow(m_btnNextSolution, m_queen->getSolutionCount() > 1);
}

void MainWindow::showPreviousSolution() {
    if (m_currentSolutionIndex > 0) {
        m_currentSolutionIndex--;
        updateDisplay();
        updateStatusLabel();
    }
}

void MainWindow::showNextSolution() {
    if (m_currentSolutionIndex < m_queen->getSolutionCount() - 1) {
        m_currentSolutionIndex++;
        updateDisplay();
        updateStatusLabel();
    }
}

void MainWindow::reset() {
    m_queen = std::make_unique<Queen>(m_boardSize);
    m_currentSolutionIndex = 0;
    m_chessBoard->clear();
    
    SetWindowTextW(m_lblSolutionInfo, L"未求解");
    SetWindowTextW(m_lblStatus, L"状态: 已重置");
    
    EnableWindow(m_btnPrevSolution, FALSE);
    EnableWindow(m_btnNextSolution, FALSE);
    
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

void MainWindow::updateDisplay() {
    if (m_queen->getSolutionCount() > 0) {
        auto solutions = m_queen->getSolutions();
        if (m_currentSolutionIndex < static_cast<int>(solutions.size())) {
            m_chessBoard->setBoard(solutions[m_currentSolutionIndex].getPositions());
            InvalidateRect(m_hwnd, nullptr, TRUE);
        }
    }
}

void MainWindow::updateStatusLabel() {
    int count = m_queen->getSolutionCount();
    if (count > 0) {
        std::wstringstream ss;
        ss << L"解: " << (m_currentSolutionIndex + 1) << L" / " << count;
        SetWindowTextW(m_lblSolutionInfo, ss.str().c_str());
    } else {
        SetWindowTextW(m_lblSolutionInfo, L"未求解");
    }
}

#endif // _WIN32
