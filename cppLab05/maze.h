#ifndef MAZE_H
#define MAZE_H

// 轻量自定义动态数组（替代 std::vector）
template<typename T>
class SimpleArray {
private:
    T* data_;
    int size_;
    int cap_;

    void grow(int minCap) {
        int newCap = cap_ == 0 ? 4 : cap_ * 2;
        if (newCap < minCap) newCap = minCap;
        T* nd = new T[newCap];
        for (int i = 0; i < size_; ++i) nd[i] = data_[i];
        delete[] data_;
        data_ = nd;
        cap_ = newCap;
    }
public:
    SimpleArray() : data_(0), size_(0), cap_(0) {}
    ~SimpleArray() { delete[] data_; }
    SimpleArray(const SimpleArray& o) : data_(0), size_(o.size_), cap_(o.cap_) {
        if (cap_ > 0) {
            data_ = new T[cap_];
            for (int i = 0; i < size_; ++i) data_[i] = o.data_[i];
        }
    }
    SimpleArray& operator=(const SimpleArray& o) {
        if (this == &o) return *this;
        delete[] data_;
        size_ = o.size_;
        cap_ = o.cap_;
        data_ = cap_ > 0 ? new T[cap_] : 0;
        for (int i = 0; i < size_; ++i) data_[i] = o.data_[i];
        return *this;
    }
    inline int size() const { return size_; }
    inline bool empty() const { return size_ == 0; }
    inline void clear() { size_ = 0; }
    inline T& operator[](int i) { return data_[i]; }
    inline const T& operator[](int i) const { return data_[i]; }
    inline T& back() { return data_[size_ - 1]; }
    inline const T& back() const { return data_[size_ - 1]; }
    void push_back(const T& v) {
        if (size_ == cap_) grow(cap_ == 0 ? 4 : cap_ * 2);
        data_[size_++] = v;
    }
    void pop_back() { if (size_ > 0) --size_; }
    void removeAtSwap(int idx) { if (idx >= 0 && idx < size_) { data_[idx] = data_[size_ - 1]; --size_; } }
};

// 点结构体,表示坐标
struct Point {
    int x;
    int y;
    
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// 路径类型：使用自定义数组
typedef SimpleArray<Point> Path;

// 单元格类,代表迷宫中的一个"块"
class Cell {
public:
    Point coords;           // 单元格坐标
    bool walls[4];          // 四个方向的墙: [0]=上, [1]=右, [2]=下, [3]=左
    bool visited;           // 求解路径时标记是否访问过
    
    Cell();
    Cell(int x, int y);
};

// 迷宫类,管理整个迷宫
class Maze {
private:
    int width;                              // 迷宫宽度 (M)
    int height;                             // 迷宫高度 (N)
    Cell* grid;                             // 单元格网格（width*height 的一维数组）
    Point start;                            // 入口坐标
    Point end;                              // 出口坐标
    
    // 私有辅助函数
    
    // 网格索引辅助
    inline int idx(int x, int y) const { return y * width + x; }
    
    // 移除两个相邻单元格之间的墙
    void removeWall(Cell& c1, Cell& c2);
    
    // 检查两个单元格是否物理相邻
    bool areAdjacent(const Cell& c1, const Cell& c2);
    
    // 获取两个相邻单元格之间的方向
    int getDirection(const Cell& from, const Cell& to);
    
    // 检查坐标是否在迷宫范围内
    bool isValid(int x, int y);
    
public:
    // 构造函数: 初始化 M x N 的全封闭迷宫
    Maze(int m, int n);
    ~Maze();
    
    // 使用随机化 Prim 算法生成完美迷宫
    void generateMaze();
    
    // 求解迷宫,返回从入口到出口的路径
    Path solve();
    
    // 打印迷宫,可选择性地高亮显示路径
    void printMaze(const Path* path = 0);

    // 只读访问接口（供 GUI 使用）
    inline int getWidth() const { return width; }
    inline int getHeight() const { return height; }
    inline Point getStart() const { return start; }
    inline Point getEnd() const { return end; }
    inline const Cell& cellAt(int x, int y) const { return grid[idx(x, y)]; }
};

#endif // MAZE_H
