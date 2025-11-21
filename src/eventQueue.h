#pragma once
#include <mutex>
#include <queue>
#include <string>

class EventQueue {
public:
    void push(const std::string &msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
    }

    bool pop(std::string &out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        out = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::mutex mutex_;
    std::queue<std::string> queue_;
};
