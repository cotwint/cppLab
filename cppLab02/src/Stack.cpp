#include "../include/Stack.h"

void Stack::push(const State& state) {
    m_data.push_back(state);
}

State Stack::pop() {
    if (m_data.empty()) {
        throw std::underflow_error("stack underflow");
    }
    State top = m_data.back();
    m_data.pop_back();
    return top;
}

bool Stack::isEmpty() const {
    return m_data.empty();
}

State Stack::peek() const {
    if (m_data.empty()) {
        throw std::underflow_error("stack is empty");
    }
    return m_data.back();
}

void Stack::clear() {
    m_data.clear();
}
