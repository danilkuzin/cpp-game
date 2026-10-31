#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ui {
class MessageLog {
public:
    static constexpr std::size_t kMaxLogLines = 4;

    void Push(std::string message);
    void PushAll(const std::vector<std::string> &messages);

    [[nodiscard]] auto begin() const { return lines_.begin(); }

    [[nodiscard]] auto end() const { return lines_.end(); }

private:
    std::vector<std::string> lines_;
};
} // namespace ui
