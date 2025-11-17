#pragma once

#include <vector>
#include <stdexcept>

struct State {
    int row{};
    int col{};
    int nextCol{};
};

class Stack {
private:
    std::vector<State> m_data;

public:
    void push(const State& state);
    State pop();
    bool isEmpty() const;
    State peek() const;
    void clear();
};
