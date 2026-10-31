#pragma once
#include <cstdint>
#include <string>
#include <variant>

namespace ui {
enum class Key : std::uint8_t {
    Unknown,
    Up,
    Down,
    Left,
    Right,
    Return,
    Escape,
    Backspace,
    Space,
    A,
    D,
    H,
    I,
    P,
    W,
    Z,
    Num1,
    Num2,
};

enum class MouseButton : std::uint8_t { Left, Middle, Right, Unknown };

struct KeyEvent {
    Key key = Key::Unknown;
};

struct TextInputEvent {
    std::string text;
};

struct MouseClickEvent {
    float x = 0.F, y = 0.F;
    MouseButton button = MouseButton::Left;
};

struct MouseMoveEvent {
    float x = 0.F, y = 0.F;
};

struct QuitEvent {};

using InputEvent =
    std::variant<KeyEvent, TextInputEvent, MouseClickEvent, MouseMoveEvent, QuitEvent>;
} // namespace ui
