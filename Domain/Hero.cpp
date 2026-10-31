#include "Hero.h"
#include "Location.h"
#include "RectInt.h"

namespace hero {

RectInt GetVisibleArea(Location location, int vis_long) {
    return RectInt{.x = location.coord.x - vis_long,
                   .y = location.coord.y - vis_long,
                   .width = (2 * vis_long) + 1,
                   .height = (2 * vis_long) + 1};
}

} // namespace hero
