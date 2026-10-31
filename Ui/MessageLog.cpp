#include "MessageLog.h"
#include <string>
#include <utility>
#include <vector>

namespace ui {
void MessageLog::Push(std::string message) {
    lines_.push_back(std::move(message));
    if (lines_.size() > kMaxLogLines) {
        lines_.erase(lines_.begin());
    }
}

void MessageLog::PushAll(const std::vector<std::string> &messages) {
    for (const auto &message : messages) {
        Push(message);
    }
}
} // namespace ui
