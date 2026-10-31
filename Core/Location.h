#pragma once

#include "Vector2Int.h"
#include <cstddef>
#include <functional>

struct Location {
    Vector2Int coord;
    int map_level = 0;

    bool operator==(const Location &other) const;
};

template <> struct std::hash<Location> {
    size_t operator()(const Location &loc) const {
        const size_t h1 = std::hash<Vector2Int>()(loc.coord);
        const size_t h2 = std::hash<int>()(loc.map_level);

        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
