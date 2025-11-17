// Win32 GUI 版入口与绘制
#include "maze.h"
#include <windows.h>
#include <tchar.h>
#include <algorithm>

#define IDC_EDIT_WIDTH   1001
#define IDC_EDIT_HEIGHT  1002
#define IDC_BTN_GEN      1003
#define IDC_BTN_SOLVE    1004

// 全局状态
static Maze*       gMaze = nullptr;
static Path        gPath;          // 最近一次求解的路径
static int         gDefaultW = 20;
static int         gDefaultH = 15;
static HWND        gHwndEditW = nullptr;
static HWND        gHwndEditH = nullptr;
static HWND        gHwndBtnGen = nullptr;
static HWND        gHwndBtnSolve = nullptr;

// 读取 Edit 文本为整数（>=2），失败返回 -1
static int ReadEditInt(HWND hEdit) {
    TCHAR buf[32];
    GetWindowText(hEdit, buf, 32);
    int v = _tstoi(buf);
    if (v < 2) return -1;
    return v;
}

// 创建/重建迷宫
static void CreateAndGenerateMaze(int w, int h) {
    if (gMaze) { delete gMaze; gMaze = nullptr; }
    gMaze = new Maze(w, h);
    gMaze->generateMaze();
    // 清空路径
    gPath.clear();
}

// 根据窗口客户区绘制迷宫
static void DrawMazeToHDC(HDC hdc, RECT rcClient) {
    if (!gMaze) return;
    const int W = gMaze->getWidth();
    const int H = gMaze->getHeight();

    // 顶部控件区域高度
    const int panelH = 48;
    const int margin = 16;
    int availW = (rcClient.right - rcClient.left) - margin * 2;
    int availH = (rcClient.bottom - rcClient.top) - panelH - margin * 2;
    if (availW <= 0 || availH <= 0) return;

    // 计算单元格像素尺寸，保持方形
    int cell = std::min(availW / W, availH / H);
    if (cell < 4) cell = 4; // 最小尺寸

    // 计算起绘点（居中）
    int mazeWpx = cell * W;
    int mazeHpx = cell * H;
    int originX = rcClient.left + (rcClient.right - rcClient.left - mazeWpx) / 2;
    int originY = rcClient.top + panelH + (rcClient.bottom - rcClient.top - panelH - mazeHpx) / 2;

    // 双缓冲
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
    HGDIOBJ oldBmp = SelectObject(memDC, bmp);

    // 背景
    HBRUSH bg = CreateSolidBrush(RGB(250, 250, 250));
    FillRect(memDC, &rcClient, bg);
    DeleteObject(bg);

    // 画标题栏下的分隔线
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    HGDIOBJ oldPen = SelectObject(memDC, gridPen);
    MoveToEx(memDC, rcClient.left, rcClient.top + panelH - 1, NULL);
    LineTo(memDC, rcClient.right, rcClient.top + panelH - 1);
    SelectObject(memDC, oldPen);
    DeleteObject(gridPen);

    // 墙体画笔
    HPEN wallPen = CreatePen(PS_SOLID, std::max(1, cell / 8), RGB(30, 30, 30));
    oldPen = SelectObject(memDC, wallPen);

    // 绘制每个格子的上、左墙，最后补最下边和最右边，避免重复
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            const Cell& c = gMaze->cellAt(x, y);
            int x0 = originX + x * cell;
            int y0 = originY + y * cell;
            int x1 = x0 + cell;
            int y1 = y0 + cell;

            if (c.walls[0]) { // 上
                MoveToEx(memDC, x0, y0, NULL);
                LineTo(memDC, x1, y0);
            }
            if (c.walls[3]) { // 左
                MoveToEx(memDC, x0, y0, NULL);
                LineTo(memDC, x0, y1);
            }
            // 最后一列的右墙
            if (x == W - 1 && c.walls[1]) {
                MoveToEx(memDC, x1, y0, NULL);
                LineTo(memDC, x1, y1);
            }
            // 最后一行的下墙
            if (y == H - 1 && c.walls[2]) {
                MoveToEx(memDC, x0, y1, NULL);
                LineTo(memDC, x1, y1);
            }
        }
    }

    // 绘制路径（粗线）
    if (!gPath.empty()) {
    HPEN pathPen = CreatePen(PS_SOLID, std::max(2, cell / 3), RGB(50, 150, 250));
        SelectObject(memDC, pathPen);
        for (int i = 0; i < gPath.size(); ++i) {
            int cx = originX + gPath[i].x * cell + cell / 2;
            int cy = originY + gPath[i].y * cell + cell / 2;
            if (i == 0) MoveToEx(memDC, cx, cy, NULL);
            else LineTo(memDC, cx, cy);
        }
        SelectObject(memDC, wallPen); // 复原到墙体笔
        DeleteObject(pathPen);
    }

    // 起点终点（圆点）
    HBRUSH startBr = CreateSolidBrush(RGB(46, 204, 113));
    HBRUSH endBr   = CreateSolidBrush(RGB(231, 76, 60));
    HBRUSH oldBr   = (HBRUSH)SelectObject(memDC, startBr);
    int r = std::max(4, cell / 4);
    Point s = gMaze->getStart();
    Point e = gMaze->getEnd();
    int sx = originX + s.x * cell + cell / 2;
    int sy = originY + s.y * cell + cell / 2;
    int ex = originX + e.x * cell + cell / 2;
    int ey = originY + e.y * cell + cell / 2;
    Ellipse(memDC, sx - r, sy - r, sx + r, sy + r);
    SelectObject(memDC, endBr);
    Ellipse(memDC, ex - r, ey - r, ex + r, ey + r);
    SelectObject(memDC, oldBr);
    DeleteObject(startBr);
    DeleteObject(endBr);

    // 收尾
    SelectObject(memDC, oldPen);
    DeleteObject(wallPen);

    BitBlt(hdc, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(bmp);
    DeleteDC(memDC);
}

static void LayoutControls(HWND hWnd) {
    RECT rc; GetClientRect(hWnd, &rc);
    int x = 12, y = 8, h = 24;
    int lblW = 72, editW = 64, btnW = 88;
    // 标签
    CreateWindowEx(0, _T("STATIC"), _T("宽度(M):"), WS_CHILD | WS_VISIBLE, x, y + 4, lblW, h, hWnd, NULL, GetModuleHandle(NULL), NULL);
    x += lblW;
    gHwndEditW = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T("20"), WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL, x, y, editW, h, hWnd, (HMENU)IDC_EDIT_WIDTH, GetModuleHandle(NULL), NULL);
    x += editW + 12;
    CreateWindowEx(0, _T("STATIC"), _T("高度(N):"), WS_CHILD | WS_VISIBLE, x, y + 4, lblW, h, hWnd, NULL, GetModuleHandle(NULL), NULL);
    x += lblW;
    gHwndEditH = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T("15"), WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL, x, y, editW, h, hWnd, (HMENU)IDC_EDIT_HEIGHT, GetModuleHandle(NULL), NULL);
    x += editW + 12;
    gHwndBtnGen = CreateWindowEx(0, _T("BUTTON"), _T("生成迷宫"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, btnW, h, hWnd, (HMENU)IDC_BTN_GEN, GetModuleHandle(NULL), NULL);
    x += btnW + 8;
    gHwndBtnSolve = CreateWindowEx(0, _T("BUTTON"), _T("求解路径"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, btnW, h, hWnd, (HMENU)IDC_BTN_SOLVE, GetModuleHandle(NULL), NULL);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        LayoutControls(hWnd);
        CreateAndGenerateMaze(gDefaultW, gDefaultH);
        return 0;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_BTN_GEN: {
            int w = ReadEditInt(gHwndEditW);
            int h = ReadEditInt(gHwndEditH);
            if (w < 2 || h < 2) {
                MessageBox(hWnd, _T("宽度和高度必须是不小于 2 的整数。"), _T("输入错误"), MB_ICONWARNING | MB_OK);
                return 0;
            }
            CreateAndGenerateMaze(w, h);
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }
        case IDC_BTN_SOLVE: {
            if (!gMaze) return 0;
            gPath = gMaze->solve();
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc; GetClientRect(hWnd, &rc);
        DrawMazeToHDC(hdc, rc);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        if (gMaze) { delete gMaze; gMaze = nullptr; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow) {
    // 注册窗口类
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = _T("MazeWin32Wnd");
    if (!RegisterClass(&wc)) return 0;

    // 初始窗口大小估计（给个合适的默认）
    int initW = 1000, initH = 700;
    HWND hWnd = CreateWindowEx(0, wc.lpszClassName, _T("基于 Prim 的迷宫 - Win32 GUI"),
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, initW, initH,
                               NULL, NULL, hInstance, NULL);
    if (!hWnd) return 0;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
