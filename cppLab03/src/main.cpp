#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <sstream>
#include <map>
#include "../include/HuffmanTree.h"
#include "../include/Application.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 控件 ID 定义
#define IDC_INPUT_TEXT 101
#define IDC_OUTPUT_TEXT 102
#define IDC_ENCODE_BTN 103
#define IDC_DECODE_BTN 104
#define IDC_COMPRESS_FILE_BTN 105
#define IDC_DECOMPRESS_FILE_BTN 106
#define IDC_STATS_TEXT 107
#define IDC_CODES_TEXT 108
#define IDC_CLEAR_BTN 109
#define IDC_TEST_BTN 110

// 全局变量
HINSTANCE g_hInst;
HWND g_hWndInputEdit, g_hWndOutputEdit, g_hWndStatsEdit, g_hWndCodesEdit;
HuffmanTree g_huffmanTree;
std::map<char, std::string> g_codes;
std::string g_originalText;
std::string g_encodedBinary;
Application g_app;

// 函数声明
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnEncodeText(HWND hwnd);
void OnDecodeText(HWND hwnd);
void OnCompressFile(HWND hwnd);
void OnDecompressFile(HWND hwnd);
void OnClear(HWND hwnd);
void OnRunTests(HWND hwnd);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    g_hInst = hInstance;

    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"HuffmanWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassW(&wc);

    // 创建窗口
    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"哈夫曼编码压缩工具",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 700,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // 创建标签和控件
        int y = 10;

        // 输入文本区域
        CreateWindowW(L"STATIC", L"输入文本:", WS_VISIBLE | WS_CHILD, 10, y, 100, 20, hwnd, nullptr, g_hInst, nullptr);
        y += 25;
        g_hWndInputEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN,
            10, y, 860, 120, hwnd, (HMENU)IDC_INPUT_TEXT, g_hInst, nullptr);
        y += 130;

        // 按钮行
        CreateWindowW(L"BUTTON", L"编码文本", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, y, 100, 30, hwnd, (HMENU)IDC_ENCODE_BTN, g_hInst, nullptr);
        CreateWindowW(L"BUTTON", L"解码验证", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            120, y, 100, 30, hwnd, (HMENU)IDC_DECODE_BTN, g_hInst, nullptr);
        CreateWindowW(L"BUTTON", L"压缩文件", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            230, y, 100, 30, hwnd, (HMENU)IDC_COMPRESS_FILE_BTN, g_hInst, nullptr);
        CreateWindowW(L"BUTTON", L"解压文件", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            340, y, 100, 30, hwnd, (HMENU)IDC_DECOMPRESS_FILE_BTN, g_hInst, nullptr);
        CreateWindowW(L"BUTTON", L"运行测试", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            450, y, 100, 30, hwnd, (HMENU)IDC_TEST_BTN, g_hInst, nullptr);
        CreateWindowW(L"BUTTON", L"清空", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            560, y, 100, 30, hwnd, (HMENU)IDC_CLEAR_BTN, g_hInst, nullptr);
        y += 40;

        // 输出文本区域
        CreateWindowW(L"STATIC", L"输出结果:", WS_VISIBLE | WS_CHILD, 10, y, 100, 20, hwnd, nullptr, g_hInst, nullptr);
        y += 25;
        g_hWndOutputEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY | ES_WANTRETURN,
            10, y, 860, 120, hwnd, (HMENU)IDC_OUTPUT_TEXT, g_hInst, nullptr);
        y += 130;

        // 统计信息
        CreateWindowW(L"STATIC", L"压缩统计:", WS_VISIBLE | WS_CHILD, 10, y, 100, 20, hwnd, nullptr, g_hInst, nullptr);
        y += 25;
        g_hWndStatsEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
            10, y, 420, 120, hwnd, (HMENU)IDC_STATS_TEXT, g_hInst, nullptr);

        // 编码表
        CreateWindowW(L"STATIC", L"编码对照表:", WS_VISIBLE | WS_CHILD, 440, y - 25, 100, 20, hwnd, nullptr, g_hInst, nullptr);
        g_hWndCodesEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
            440, y, 430, 120, hwnd, (HMENU)IDC_CODES_TEXT, g_hInst, nullptr);

        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_ENCODE_BTN:
            OnEncodeText(hwnd);
            break;
        case IDC_DECODE_BTN:
            OnDecodeText(hwnd);
            break;
        case IDC_COMPRESS_FILE_BTN:
            OnCompressFile(hwnd);
            break;
        case IDC_DECOMPRESS_FILE_BTN:
            OnDecompressFile(hwnd);
            break;
        case IDC_CLEAR_BTN:
            OnClear(hwnd);
            break;
        case IDC_TEST_BTN:
            OnRunTests(hwnd);
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void OnEncodeText(HWND hwnd) {
    // 获取输入文本
    int len = GetWindowTextLengthW(g_hWndInputEdit);
    if (len == 0) {
        MessageBoxW(hwnd, L"请输入要编码的文本！", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    std::wstring wtext(len + 1, 0);
    GetWindowTextW(g_hWndInputEdit, &wtext[0], len + 1);
    wtext.resize(len);

    g_originalText = WStringToString(wtext);

    try {
        // 执行编码
        g_codes = g_huffmanTree.encode(g_originalText);
        g_encodedBinary = g_huffmanTree.encodeText(g_originalText, g_codes);

        // 显示编码结果（显示前100个字符）
        std::string displayBinary = g_encodedBinary.substr(0, std::min<size_t>(500, g_encodedBinary.size()));
        if (g_encodedBinary.size() > 500) {
            displayBinary += "\r\n... (总共 " + std::to_string(g_encodedBinary.size()) + " 位)";
        }
        SetWindowTextW(g_hWndOutputEdit, StringToWString(displayBinary).c_str());

        // 显示统计信息
        std::ostringstream stats;
        stats << "原始字符数: " << g_originalText.size() << "\r\n";
        stats << "原始比特数: " << (g_originalText.size() * 8) << "\r\n";
        stats << "编码后比特数: " << g_encodedBinary.size() << "\r\n";
        double ratio = static_cast<double>(g_encodedBinary.size()) / (g_originalText.size() * 8);
        stats << "压缩率: " << (ratio * 100) << "%\r\n";
        stats << "节省空间: " << ((1 - ratio) * 100) << "%";
        SetWindowTextW(g_hWndStatsEdit, StringToWString(stats.str()).c_str());

        // 显示编码表
        std::ostringstream codes;
        for (const auto& entry : g_codes) {
            if (entry.first == '\r') {
                codes << "'\\r'";
            } else if (entry.first == '\n') {
                codes << "'\\n'";
            } else if (entry.first == '\t') {
                codes << "'\\t'";
            } else if (entry.first == ' ') {
                codes << "' '";
            } else {
                codes << "'" << entry.first << "'";
            }
            codes << " -> " << entry.second << "\r\n";
        }
        SetWindowTextW(g_hWndCodesEdit, StringToWString(codes.str()).c_str());

        MessageBoxW(hwnd, L"编码完成！", L"成功", MB_OK | MB_ICONINFORMATION);
    }
    catch (const std::exception& ex) {
        MessageBoxW(hwnd, StringToWString("编码失败: " + std::string(ex.what())).c_str(), L"错误", MB_OK | MB_ICONERROR);
    }
}

void OnDecodeText(HWND hwnd) {
    if (g_encodedBinary.empty()) {
        MessageBoxW(hwnd, L"请先进行编码操作！", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    try {
        std::string decoded = g_huffmanTree.decodeText(g_encodedBinary);
        SetWindowTextW(g_hWndOutputEdit, StringToWString(decoded).c_str());

        if (decoded == g_originalText) {
            MessageBoxW(hwnd, L"解码成功！结果与原文本一致。", L"验证成功", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBoxW(hwnd, L"警告：解码结果与原文本不一致！", L"验证失败", MB_OK | MB_ICONWARNING);
        }
    }
    catch (const std::exception& ex) {
        MessageBoxW(hwnd, StringToWString("解码失败: " + std::string(ex.what())).c_str(), L"错误", MB_OK | MB_ICONERROR);
    }
}

void OnCompressFile(HWND hwnd) {
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"所有文件\0*.*\0文本文件\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        std::string inputPath = WStringToString(ofn.lpstrFile);

        // 选择输出文件
        wchar_t szOutputFile[260] = { 0 };
        wcscpy_s(szOutputFile, ofn.lpstrFile);
        wcscat_s(szOutputFile, L".huf");

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szOutputFile;
        ofn.nMaxFile = sizeof(szOutputFile);
        ofn.lpstrFilter = L"压缩文件\0*.HUF\0所有文件\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn) == TRUE) {
            std::string outputPath = WStringToString(ofn.lpstrFile);

            try {
                g_app.compressFile(inputPath, outputPath);
                MessageBoxW(hwnd, L"文件压缩成功！", L"成功", MB_OK | MB_ICONINFORMATION);
            }
            catch (const std::exception& ex) {
                MessageBoxW(hwnd, StringToWString("压缩失败: " + std::string(ex.what())).c_str(), L"错误", MB_OK | MB_ICONERROR);
            }
        }
    }
}

void OnDecompressFile(HWND hwnd) {
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"压缩文件\0*.HUF\0所有文件\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        std::string inputPath = WStringToString(ofn.lpstrFile);

        // 选择输出文件
        wchar_t szOutputFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szOutputFile;
        ofn.nMaxFile = sizeof(szOutputFile);
        ofn.lpstrFilter = L"所有文件\0*.*\0文本文件\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn) == TRUE) {
            std::string outputPath = WStringToString(ofn.lpstrFile);

            try {
                g_app.decompressFile(inputPath, outputPath);
                MessageBoxW(hwnd, L"文件解压成功！", L"成功", MB_OK | MB_ICONINFORMATION);
            }
            catch (const std::exception& ex) {
                MessageBoxW(hwnd, StringToWString("解压失败: " + std::string(ex.what())).c_str(), L"错误", MB_OK | MB_ICONERROR);
            }
        }
    }
}

void OnClear(HWND hwnd) {
    SetWindowTextW(g_hWndInputEdit, L"");
    SetWindowTextW(g_hWndOutputEdit, L"");
    SetWindowTextW(g_hWndStatsEdit, L"");
    SetWindowTextW(g_hWndCodesEdit, L"");
    g_originalText.clear();
    g_encodedBinary.clear();
    g_codes.clear();
}

void OnRunTests(HWND hwnd) {
    std::ostringstream output;
    const std::vector<std::pair<std::string, std::string>> testCases = {
        {"测试1", "hello world"},
        {"测试2", "the quick brown fox jumps over the lazy dog"},
        {"测试3", "aaaaaaaaabbbbbbcccdde"}
    };

    for (const auto& [name, text] : testCases) {
        HuffmanTree tree;
        auto codes = tree.encode(text);
        std::string encoded = tree.encodeText(text, codes);
        std::string decoded = tree.decodeText(encoded);

        output << name << ":\r\n";
        output << "原文: " << text << "\r\n";
        output << "编码长度: " << encoded.size() << " 位\r\n";
        output << "压缩率: " << (static_cast<double>(encoded.size()) / (text.size() * 8) * 100) << "%\r\n";
        output << "验证: " << (decoded == text ? "通过" : "失败") << "\r\n\r\n";
    }

    SetWindowTextW(g_hWndOutputEdit, StringToWString(output.str()).c_str());
    MessageBoxW(hwnd, L"测试完成！结果已显示在输出区域。", L"测试", MB_OK | MB_ICONINFORMATION);
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}
