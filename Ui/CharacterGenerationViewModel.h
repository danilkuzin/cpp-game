#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ui {
struct CharacterGenerationViewModel {
    std::string title;
    std::string prompt;

    enum class Mode : std::uint8_t { SelectingOption, TextInput } mode = Mode::SelectingOption;

    std::vector<std::string> options;
    size_t selected_idx = 0;

    std::string input_text;
    std::vector<std::string> details;
};
} // namespace ui
