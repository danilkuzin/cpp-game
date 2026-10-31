#pragma once

#include "Vector2Int.h"

struct RectInt {
    int x = 0, y = 0, width = 0, height = 0;

    [[nodiscard]] bool ContainsPoint(Vector2Int coord) const;
};
