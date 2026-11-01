#include "Location.h"

bool Location::operator==(const Location &other) const {
    return coord == other.coord && map_level == other.map_level;
}

Location Location::operator+(const Vector2Int offset) const {
    return Location{.coord = coord + offset, .map_level = map_level};
}
