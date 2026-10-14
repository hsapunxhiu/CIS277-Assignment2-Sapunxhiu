#ifndef STACK_H
#define STACK_H

#include <vector>
#include <stdexcept>
#include <cstddef>

template <typename T>
class Stack {
private:
    std::vector<T> data;

public:
    Stack() = default;

    void push(const T& value) {
        data.push_back(value);
    }

    T pop() {
        if (empty()) {
            throw std::underflow_error("Stack underflow: cannot pop from an empty stack.");
        }
        T topValue = data.back();
        data.pop_back();
        return topValue;
    }

    T& top() {
        if (empty()) {
            throw std::out_of_range("Stack is empty: cannot access top element.");
        }
        return data.back();
    }

    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Stack is empty: cannot access top element.");
        }
        return data.back();
    }

    bool empty() const {
        return data.empty();
    }

    std::size_t size() const {
        return data.size();
    }
};

#endif // STACK_H