#include "Location.h"

bool Location::operator==(const Location &other) const {
    return coord == other.coord && map_level == other.map_level;
}
