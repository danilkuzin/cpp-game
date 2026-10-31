#include "RectInt.h"
#include "Vector2Int.h"

bool RectInt::ContainsPoint(const Vector2Int coord) const {
    return coord.x >= x && coord.x < x + width && coord.y >= y && coord.y < y + height;
}
