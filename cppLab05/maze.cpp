#include "maze.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

// 简单线性同余随机数生成器（避免使用 <random>）
class SimpleRNG {
private:
    unsigned int state;
public:
    SimpleRNG(unsigned int seed = 0) { if (seed == 0) seed = (unsigned int)std::time(0); state = seed; }
    unsigned int next() { state = (1103515245u * state + 12345u); return state; }
    int nextInt(int max) { return (int)(next() % (unsigned int)max); }
};

// Cell 类实现

Cell::Cell() : coords(0, 0), visited(false) {
    // 初始化所有墙都存在
    for (int i = 0; i < 4; i++) {
        walls[i] = true;
    }
}

Cell::Cell(int x, int y) : coords(x, y), visited(false) {
    // 初始化所有墙都存在
    for (int i = 0; i < 4; i++) {
        walls[i] = true;
    }
}

// Maze 类实现

Maze::Maze(int m, int n) : width(m), height(n), grid(0) {
    grid = new Cell[width * height];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[idx(x, y)] = Cell(x, y);
        }
    }
    start = Point(0, 0);
    end = Point(width - 1, height - 1);
}

Maze::~Maze() { delete[] grid; }



// 检查两个单元格是否物理相邻
bool Maze::areAdjacent(const Cell& c1, const Cell& c2) {
    int dx = c1.coords.x - c2.coords.x;
    int dy = c1.coords.y - c2.coords.y;
    if (dx == 0 && (dy == 1 || dy == -1)) return true;
    if (dy == 0 && (dx == 1 || dx == -1)) return true;
    return false;
}

// 获取从from到to的方向: 0=上, 1=右, 2=下, 3=左
int Maze::getDirection(const Cell& from, const Cell& to) {
    if (to.coords.y < from.coords.y) return 0; // 上
    if (to.coords.x > from.coords.x) return 1; // 右
    if (to.coords.y > from.coords.y) return 2; // 下
    if (to.coords.x < from.coords.x) return 3; // 左
    return -1;
}

// 移除两个相邻单元格之间的墙
void Maze::removeWall(Cell& c1, Cell& c2) {
    int dir = getDirection(c1, c2);
    if (dir == -1) return;
    
    // 移除c1朝向c2方向的墙
    c1.walls[dir] = false;
    
    // 移除c2朝向c1方向的墙 (相反方向)
    int oppositeDir = (dir + 2) % 4;
    c2.walls[oppositeDir] = false;
}

// 基于希尔伯特曲线生成迷宫
// 使用随机化 Prim 算法生成完美迷宫
void Maze::generateMaze() {
    // 标记数组：是否已在树中
    SimpleArray<int> inTree; // 0/1 标记
    inTree.clear();
    // 预分配
    for (int i = 0; i < width * height; ++i) inTree.push_back(0);

    // Frontier 边结构
    struct Edge { int x1,y1,x2,y2; };
    SimpleArray<Edge> frontier;

    SimpleRNG rng((unsigned int)std::time(0));

    // 起点固定 start
    inTree[idx(start.x, start.y)] = 1;

    // 将起点的邻居加入 frontier
    int dx[4] = {0,1,0,-1};
    int dy[4] = {-1,0,1,0};
    for (int d = 0; d < 4; ++d) {
        int nx = start.x + dx[d];
        int ny = start.y + dy[d];
        if (isValid(nx, ny)) frontier.push_back(Edge{start.x,start.y,nx,ny});
    }

    // Prim 过程
    while (!frontier.empty()) {
        int pick = rng.nextInt(frontier.size());
        Edge e = frontier[pick];
        frontier.removeAtSwap(pick);

        int idx1 = idx(e.x1, e.y1);
        int idx2 = idx(e.x2, e.y2);
        bool in1 = inTree[idx1] != 0;
        bool in2 = inTree[idx2] != 0;
        // 需要恰好一个端点在树中
        if (in1 ^ in2) {
            Cell& c1 = grid[idx(e.x1, e.y1)];
            Cell& c2 = grid[idx(e.x2, e.y2)];
            removeWall(c1, c2);
            // 新加入的端点
            int nx, ny;
            if (!in1) { nx = e.x1; ny = e.y1; inTree[idx1] = 1; }
            else { nx = e.x2; ny = e.y2; inTree[idx2] = 1; }
            // 添加它的邻居边
            for (int d = 0; d < 4; ++d) {
                int tx = nx + dx[d];
                int ty = ny + dy[d];
                if (isValid(tx, ty) && inTree[idx(tx, ty)] == 0) {
                    frontier.push_back(Edge{nx,ny,tx,ty});
                }
            }
        }
    }

    // 打开入口和出口外墙（各自开一面：入口开左，上优先；出口开右，下优先）
    // 入口 (0,0)
    grid[idx(0,0)].walls[3] = false; // 左
    grid[idx(0,0)].walls[0] = false; // 上
    // 出口 (width-1,height-1)
    grid[idx(width-1,height-1)].walls[1] = false; // 右
    grid[idx(width-1,height-1)].walls[2] = false; // 下
}

// 检查坐标是否在迷宫范围内
bool Maze::isValid(int x, int y) { return x >= 0 && x < width && y >= 0 && y < height; }

// 使用栈实现的DFS回溯算法求解迷宫
Path Maze::solve() {
    // 清访标记
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            grid[idx(x,y)].visited = false;

    // 手写栈（容量 width*height）
    Point* stack = new Point[width * height];
    int top = 0;
    stack[top++] = start;
    grid[idx(start.x,start.y)].visited = true;

    // 前驱数组
    Point* parent = new Point[width * height];
    for (int i = 0; i < width * height; ++i) parent[i] = Point(-1,-1);

    int dx[4] = {0,1,0,-1};
    int dy[4] = {-1,0,1,0};
    bool found = false;

    while (top > 0 && !found) {
        Point current = stack[top - 1];
        if (current == end) { found = true; break; }
        bool advanced = false;
        for (int dir = 0; dir < 4; ++dir) {
            if (grid[idx(current.x,current.y)].walls[dir]) continue;
            int nx = current.x + dx[dir];
            int ny = current.y + dy[dir];
            if (isValid(nx,ny) && !grid[idx(nx,ny)].visited) {
                grid[idx(nx,ny)].visited = true;
                parent[idx(nx,ny)] = current;
                stack[top++] = Point(nx,ny);
                advanced = true;
                break;
            }
        }
        if (!advanced) --top; // 回溯
    }

    Path path;
    if (found) {
        // 反向收集
        Point cur = end;
        SimpleArray<Point> rev;
        while (!(cur == Point(-1,-1))) {
            rev.push_back(cur);
            if (cur == start) break;
            cur = parent[idx(cur.x,cur.y)];
        }
        // 翻转到 path
        for (int i = rev.size() - 1; i >= 0; --i) path.push_back(rev[i]);
    }

    delete[] stack;
    delete[] parent;
    return path;
}

// 打印迷宫
void Maze::printMaze(const Path* path) {
    // 若提供路径则建立查找标记数组
    SimpleArray<int> onPath;
    if (path) {
        onPath.clear();
        for (int i = 0; i < width * height; ++i) onPath.push_back(0);
        for (int i = 0; i < path->size(); ++i) {
            const Point& p = (*path)[i];
            onPath[idx(p.x,p.y)] = 1;
        }
    }

    for (int y = 0; y < height; ++y) {
        // 上边
        for (int x = 0; x < width; ++x) {
            std::cout << "+";
            if (grid[idx(x,y)].walls[0]) std::cout << "---"; else std::cout << "   ";
        }
        std::cout << "+" << std::endl;
        // 中间行
        for (int x = 0; x < width; ++x) {
            if (grid[idx(x,y)].walls[3]) std::cout << "|"; else std::cout << " ";
            Point current(x,y);
            if (current == start) std::cout << " S ";
            else if (current == end) std::cout << " E ";
            else if (path && onPath[idx(x,y)]) std::cout << " * ";
            else std::cout << "   ";
        }
        if (grid[idx(width-1,y)].walls[1]) std::cout << "|"; else std::cout << " ";
        std::cout << std::endl;
    }
    for (int x = 0; x < width; ++x) {
        std::cout << "+";
        if (grid[idx(x,height-1)].walls[2]) std::cout << "---"; else std::cout << "   ";
    }
    std::cout << "+" << std::endl;
}
