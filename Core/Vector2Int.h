#pragma once

#include <array>
#include <cstddef>
#include <functional>

struct Vector2Int {
    int x = 0, y = 0;
    Vector2Int operator+(const Vector2Int &other) const;
    Vector2Int operator+=(const Vector2Int &other);
    Vector2Int operator-(const Vector2Int &other) const;
    Vector2Int operator-=(const Vector2Int &other);
    Vector2Int operator*(int other) const;

    bool operator==(const Vector2Int &other) const;
    [[nodiscard]] int DistanceTo(Vector2Int coord) const;

    static Vector2Int Left();
    static Vector2Int Right();
    static Vector2Int Up();
    static Vector2Int Down();

    static std::array<Vector2Int, 4> Cardinals();
};

template <> struct std::hash<Vector2Int> {
    size_t operator()(const Vector2Int &v) const {
        const size_t h1 = std::hash<int>()(v.x);
        const size_t h2 = std::hash<int>()(v.y);

        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
