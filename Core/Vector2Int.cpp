#include "Vector2Int.h"
#include <array>
#include <cmath>

Vector2Int Vector2Int::operator+(const Vector2Int &other) const {
    return Vector2Int{.x = x + other.x, .y = y + other.y};
}

Vector2Int Vector2Int::operator+=(const Vector2Int &other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2Int Vector2Int::operator-(const Vector2Int &other) const {
    return Vector2Int{.x = x - other.x, .y = y - other.y};
}

Vector2Int Vector2Int::operator-=(const Vector2Int &other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2Int Vector2Int::operator*(int other) const {
    return Vector2Int{.x = x * other, .y = y * other};
}

Vector2Int Vector2Int::Left() { return Vector2Int{.x = -1, .y = 0}; }

Vector2Int Vector2Int::Right() { return Vector2Int{.x = 1, .y = 0}; }

Vector2Int Vector2Int::Up() { return Vector2Int{.x = 0, .y = -1}; }

Vector2Int Vector2Int::Down() { return Vector2Int{.x = 0, .y = 1}; }

std::array<Vector2Int, 4> Vector2Int::Cardinals() {
    return {Vector2Int::Up(), Vector2Int::Right(), Vector2Int::Down(), Vector2Int::Left()};
}

bool Vector2Int::operator==(const Vector2Int &other) const { return x == other.x && y == other.y; }

int Vector2Int::DistanceTo(const Vector2Int coord) const {
    return abs(x - coord.x) + abs(y - coord.y);
}
