#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")

using namespace std;

// 顺序表（SequenceList）类：使用动态数组实现，可自动扩容
class SequenceList {
private:
    int *m_data;      // 数据数组指针
    int m_size;       // 当前元素个数
    int m_capacity;   // 数组容量

    // 扩容函数（私有）: 当容量不足时调用，容量翻倍
    void expandCapacity() {
        int newCap = (m_capacity == 0) ? 1 : m_capacity * 2;
        int *newData = new int[newCap];
        for (int i = 0; i < m_size; ++i) {
            newData[i] = m_data[i];
        }
        delete[] m_data;
        m_data = newData;
        m_capacity = newCap;
    }

public:
    // 构造函数：默认容量可以由用户指定
    SequenceList(int capacity = 10)
        : m_data(nullptr), m_size(0), m_capacity(0) {
        if (capacity < 0) capacity = 10;
        m_data = new int[capacity];
        m_capacity = capacity;
        m_size = 0;
    }

    // 析构函数：释放内存
    ~SequenceList() {
        delete[] m_data;
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    // 插入元素到表尾，遇到容量不足自动扩容
    void insert(int value) {
        if (m_size >= m_capacity) {
            expandCapacity();
        }
        m_data[m_size++] = value;
    }

    // 显示所有元素（以空格分隔）
    void display() const {
        if (m_size == 0) {
            cout << "SequenceList is empty." << endl;
            return;
        }
        for (int i = 0; i < m_size; ++i) {
            cout << m_data[i];
            if (i != m_size - 1) cout << " ";
        }
        cout << endl;
    }

    // 获取列表大小
    int getSize() const {
        return m_size;
    }

    // 获取指定位置元素（0-based 索引）。返回 true 表示成功，false 表示索引越界
    bool getElement(int pos, int &out) const {
        if (pos < 0 || pos >= m_size) return false;
        out = m_data[pos];
        return true;
    }

    // 获取内部数组指针（允许 QuickSort 操作原始数组）。
    // 为了效率和演示目的，这里返回非 const 指针。
    int* data() const {
        return m_data;
    }

    // 交换两个位置的元素（用于封装数组操作）
    void swapElements(int i, int j) {
        if (i < 0 || i >= m_size || j < 0 || j >= m_size) return;
        int tmp = m_data[i];
        m_data[i] = m_data[j];
        m_data[j] = tmp;
    }
};

// 快速排序与快速选择算法封装类
class QuickSort {
public:
    // 分区函数（Lomuto partition 风格）
    // 将 arr[left..right] 分区，返回基准最终位置的索引
    static int partition(int arr[], int left, int right) {
        // 选择最右边元素作为基准
        int pivot = arr[right];
        int i = left - 1; // 小于等于 pivot 的区间末端
        for (int j = left; j < right; ++j) {
            if (arr[j] <= pivot) {
                ++i;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[right]);
        return i + 1;
    }

    // 随机化分区（用于避免最坏情况），返回基准位置
    static int randomizedPartition(int arr[], int left, int right) {
        int pivotIndex = left + rand() % (right - left + 1);
        swap(arr[pivotIndex], arr[right]);
        return partition(arr, left, right);
    }

    // 快速排序主函数（递归）
    static void quickSort(int arr[], int left, int right) {
        if (left < right) {
            int pivotIndex = partition(arr, left, right);
            quickSort(arr, left, pivotIndex - 1);
            quickSort(arr, pivotIndex + 1, right);
        }
    }

    // 快速选择：查找第 k 小元素（k 为 1-based）
    // 使用随机化 partition 来保证期望线性时间
    static int selectKth(int arr[], int left, int right, int k) {
        if (left > right || k <= 0 || k > (right - left + 1)) {
            throw out_of_range("k is out of range in selectKth");
        }
        // 当区间只有一个元素时，直接返回
        if (left == right) return arr[left];

        // 使用随机化分区
        int pivotIndex = randomizedPartition(arr, left, right);
        int rank = pivotIndex - left + 1; // pivot 在区间内是第 rank 小

        if (k == rank) {
            return arr[pivotIndex];
        } else if (k < rank) {
            return selectKth(arr, left, pivotIndex - 1, k);
        } else {
            return selectKth(arr, pivotIndex + 1, right, k - rank);
        }
    }

    // 包装函数：对 SequenceList 进行快速排序
    static void sortSequenceList(SequenceList &list) {
        int n = list.getSize();
        if (n <= 1) return;
        quickSort(list.data(), 0, n - 1);
    }

    // 包装函数：在不改变原顺序（复制数组）的前提下查找第 k 小元素
    // 如果希望原地查找直接使用 selectKth(list.data(), ...)
    static int selectKthFromSequenceList(const SequenceList &list, int k) {
        int n = list.getSize();
        if (k <= 0 || k > n) throw out_of_range("k is out of range");
        // 复制数组以免修改原始顺序表
        int *copyArr = new int[n];
        for (int i = 0; i < n; ++i) copyArr[i] = list.data()[i];
        // 使用非类静态 selectKth 需要 cast 去除 const 问题
        int result = selectKth(copyArr, 0, n - 1, k);
        delete[] copyArr;
        return result;
    }
};

// GUI 控件 ID 定义
#define IDC_INPUT_EDIT      101
#define IDC_ADD_BUTTON      102
#define IDC_CLEAR_BUTTON    103
#define IDC_SORT_BUTTON     104
#define IDC_FIND_KTH_BUTTON 105
#define IDC_RANDOM_BUTTON   106
#define IDC_K_EDIT          107
#define IDC_ORIGINAL_LIST   108
#define IDC_SORTED_LIST     109
#define IDC_RESULT_EDIT     110
#define IDC_STATUS_BAR      111

// 全局变量
HINSTANCE hInst;
SequenceList* g_sequence = nullptr;
SequenceList* g_sortedSequence = nullptr;

// 辅助函数：将整数向量转换为字符串显示
wstring vectorToWString(const vector<int>& vec) {
    wstringstream wss;
    for (size_t i = 0; i < vec.size(); ++i) {
        wss << vec[i];
        if (i < vec.size() - 1) wss << L", ";
    }
    return wss.str();
}

// 辅助函数：从 SequenceList 获取所有元素到 vector
vector<int> sequenceToVector(const SequenceList& list) {
    vector<int> result;
    for (int i = 0; i < list.getSize(); ++i) {
        int val;
        if (list.getElement(i, val)) {
            result.push_back(val);
        }
    }
    return result;
}

// 更新列表框显示
void UpdateListDisplay(HWND hwnd) {
    HWND hOriginal = GetDlgItem(hwnd, IDC_ORIGINAL_LIST);
    HWND hSorted = GetDlgItem(hwnd, IDC_SORTED_LIST);
    
    // 清空列表框
    SendMessage(hOriginal, LB_RESETCONTENT, 0, 0);
    SendMessage(hSorted, LB_RESETCONTENT, 0, 0);
    
    // 更新原始列表
    if (g_sequence && g_sequence->getSize() > 0) {
        vector<int> vec = sequenceToVector(*g_sequence);
        wstring str = vectorToWString(vec);
        SendMessage(hOriginal, LB_ADDSTRING, 0, (LPARAM)str.c_str());
    } else {
        SendMessage(hOriginal, LB_ADDSTRING, 0, (LPARAM)L"(空)");
    }
    
    // 更新排序后列表
    if (g_sortedSequence && g_sortedSequence->getSize() > 0) {
        vector<int> vec = sequenceToVector(*g_sortedSequence);
        wstring str = vectorToWString(vec);
        SendMessage(hSorted, LB_ADDSTRING, 0, (LPARAM)str.c_str());
    } else {
        SendMessage(hSorted, LB_ADDSTRING, 0, (LPARAM)L"(未排序)");
    }
}

// 更新状态栏
void UpdateStatusBar(HWND hwnd, const wstring& message) {
    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS_BAR);
    SetWindowText(hStatus, message.c_str());
}

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 创建输入框标签
            CreateWindowW(L"STATIC", L"输入整数（逗号或空格分隔）:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 10, 250, 20, hwnd, NULL, hInst, NULL);
            
            // 创建输入框
            CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
                10, 35, 400, 25, hwnd, (HMENU)IDC_INPUT_EDIT, hInst, NULL);
            
            // 创建添加按钮
            CreateWindowW(L"BUTTON", L"添加数字",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                420, 35, 100, 25, hwnd, (HMENU)IDC_ADD_BUTTON, hInst, NULL);
            
            // 创建随机生成按钮
            CreateWindowW(L"BUTTON", L"随机生成10个",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                530, 35, 120, 25, hwnd, (HMENU)IDC_RANDOM_BUTTON, hInst, NULL);
            
            // 创建清空按钮
            CreateWindowW(L"BUTTON", L"清空列表",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                660, 35, 100, 25, hwnd, (HMENU)IDC_CLEAR_BUTTON, hInst, NULL);
            
            // 原始列表标签
            CreateWindowW(L"STATIC", L"原始数据:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 75, 100, 20, hwnd, NULL, hInst, NULL);
            
            // 原始列表框
            CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
                10, 100, 370, 80, hwnd, (HMENU)IDC_ORIGINAL_LIST, hInst, NULL);
            
            // 排序后列表标签
            CreateWindowW(L"STATIC", L"排序后数据:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                390, 75, 100, 20, hwnd, NULL, hInst, NULL);
            
            // 排序后列表框
            CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
                390, 100, 370, 80, hwnd, (HMENU)IDC_SORTED_LIST, hInst, NULL);
            
            // K值输入标签
            CreateWindowW(L"STATIC", L"查找第K小元素，K =",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 195, 150, 20, hwnd, NULL, hInst, NULL);
            
            // K值输入框
            CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1",
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
                165, 192, 60, 25, hwnd, (HMENU)IDC_K_EDIT, hInst, NULL);
            
            // 查找按钮
            CreateWindowW(L"BUTTON", L"查找第K小",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                235, 192, 100, 25, hwnd, (HMENU)IDC_FIND_KTH_BUTTON, hInst, NULL);
            
            // 排序按钮
            CreateWindowW(L"BUTTON", L"快速排序",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                345, 192, 100, 25, hwnd, (HMENU)IDC_SORT_BUTTON, hInst, NULL);
            
            // 结果显示标签
            CreateWindowW(L"STATIC", L"结果:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 230, 100, 20, hwnd, NULL, hInst, NULL);
            
            // 结果显示框
            CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                10, 255, 750, 80, hwnd, (HMENU)IDC_RESULT_EDIT, hInst, NULL);
            
            // 状态栏
            CreateWindowExW(0, L"STATIC", L"就绪",
                WS_CHILD | WS_VISIBLE | SS_LEFT | SS_SUNKEN,
                0, 345, 800, 25, hwnd, (HMENU)IDC_STATUS_BAR, hInst, NULL);
            
            // 初始化数据结构
            g_sequence = new SequenceList(20);
            g_sortedSequence = new SequenceList(20);
            
            UpdateListDisplay(hwnd);
            break;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_ADD_BUTTON: {
                    wchar_t buffer[1024];
                    GetDlgItemText(hwnd, IDC_INPUT_EDIT, buffer, 1024);
                    wstring input(buffer);
                    
                    if (input.empty()) {
                        UpdateStatusBar(hwnd, L"请输入数字！");
                        MessageBoxW(hwnd, L"请输入数字！", L"提示", MB_OK | MB_ICONWARNING);
                        break;
                    }
                    
                    // 解析输入
                    vector<int> numbers;
                    wstringstream wss(input);
                    wstring token;
                    
                    while (wss >> token) {
                        // 移除逗号
                        token.erase(remove(token.begin(), token.end(), L','), token.end());
                        if (!token.empty()) {
                            try {
                                int num = stoi(token);
                                numbers.push_back(num);
                            } catch (...) {
                                wstring msg = L"无法解析数字: " + token;
                                MessageBoxW(hwnd, msg.c_str(), L"错误", MB_OK | MB_ICONERROR);
                            }
                        }
                    }
                    
                    // 添加到序列
                    for (int num : numbers) {
                        g_sequence->insert(num);
                    }
                    
                    // 清空排序列表
                    delete g_sortedSequence;
                    g_sortedSequence = new SequenceList(20);
                    
                    UpdateListDisplay(hwnd);
                    SetDlgItemText(hwnd, IDC_INPUT_EDIT, L"");
                    
                    wstringstream status;
                    status << L"已添加 " << numbers.size() << L" 个数字，总共 " << g_sequence->getSize() << L" 个元素";
                    UpdateStatusBar(hwnd, status.str());
                    break;
                }
                
                case IDC_RANDOM_BUTTON: {
                    for (int i = 0; i < 10; ++i) {
                        int num = rand() % 100;
                        g_sequence->insert(num);
                    }
                    
                    // 清空排序列表
                    delete g_sortedSequence;
                    g_sortedSequence = new SequenceList(20);
                    
                    UpdateListDisplay(hwnd);
                    UpdateStatusBar(hwnd, L"已随机生成10个数字");
                    break;
                }
                
                case IDC_CLEAR_BUTTON: {
                    delete g_sequence;
                    delete g_sortedSequence;
                    g_sequence = new SequenceList(20);
                    g_sortedSequence = new SequenceList(20);
                    
                    UpdateListDisplay(hwnd);
                    SetDlgItemText(hwnd, IDC_RESULT_EDIT, L"");
                    UpdateStatusBar(hwnd, L"已清空所有数据");
                    break;
                }
                
                case IDC_SORT_BUTTON: {
                    if (g_sequence->getSize() == 0) {
                        MessageBoxW(hwnd, L"列表为空，无法排序！", L"提示", MB_OK | MB_ICONWARNING);
                        break;
                    }
                    
                    // 复制原始数据到排序列表
                    delete g_sortedSequence;
                    g_sortedSequence = new SequenceList(g_sequence->getSize());
                    for (int i = 0; i < g_sequence->getSize(); ++i) {
                        int val;
                        g_sequence->getElement(i, val);
                        g_sortedSequence->insert(val);
                    }
                    
                    // 执行快速排序
                    DWORD startTime = GetTickCount();
                    QuickSort::sortSequenceList(*g_sortedSequence);
                    DWORD endTime = GetTickCount();
                    
                    UpdateListDisplay(hwnd);
                    
                    wstringstream result;
                    result << L"排序完成！\r\n";
                    result << L"元素数量: " << g_sortedSequence->getSize() << L"\r\n";
                    result << L"用时: " << (endTime - startTime) << L" 毫秒";
                    SetDlgItemText(hwnd, IDC_RESULT_EDIT, result.str().c_str());
                    UpdateStatusBar(hwnd, L"排序完成");
                    break;
                }
                
                case IDC_FIND_KTH_BUTTON: {
                    if (g_sequence->getSize() == 0) {
                        MessageBoxW(hwnd, L"列表为空，无法查找！", L"提示", MB_OK | MB_ICONWARNING);
                        break;
                    }
                    
                    wchar_t kBuffer[32];
                    GetDlgItemText(hwnd, IDC_K_EDIT, kBuffer, 32);
                    int k = _wtoi(kBuffer);
                    
                    if (k <= 0 || k > g_sequence->getSize()) {
                        wstringstream msg;
                        msg << L"K 值必须在 1 到 " << g_sequence->getSize() << L" 之间！";
                        MessageBoxW(hwnd, msg.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
                        break;
                    }
                    
                    try {
                        DWORD startTime = GetTickCount();
                        int kthValue = QuickSort::selectKthFromSequenceList(*g_sequence, k);
                        DWORD endTime = GetTickCount();
                        
                        wstringstream result;
                        result << L"查找第 " << k << L" 小元素:\r\n";
                        result << L"结果: " << kthValue << L"\r\n";
                        result << L"用时: " << (endTime - startTime) << L" 毫秒\r\n\r\n";
                        result << L"说明: 如果将数据从小到大排序，第 " << k << L" 个位置的元素是 " << kthValue;
                        
                        SetDlgItemText(hwnd, IDC_RESULT_EDIT, result.str().c_str());
                        UpdateStatusBar(hwnd, L"查找完成");
                    } catch (const exception& e) {
                        MessageBoxA(hwnd, e.what(), "错误", MB_OK | MB_ICONERROR);
                    }
                    break;
                }
            }
            break;
        }
        
        case WM_DESTROY: {
            delete g_sequence;
            delete g_sortedSequence;
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// WinMain 入口函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    srand((unsigned)time(nullptr));
    
    // 注册窗口类
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"QuickSortWindowClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"窗口注册失败！", L"错误", MB_ICONERROR | MB_OK);
        return 0;
    }
    
    // 创建窗口
    HWND hwnd = CreateWindowExW(
        0,
        L"QuickSortWindowClass",
        L"快速排序与第K小元素查找",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 420,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) {
        MessageBoxW(NULL, L"窗口创建失败！", L"错误", MB_ICONERROR | MB_OK);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
