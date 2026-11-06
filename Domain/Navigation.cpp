#include "Navigation.h"
#include "Location.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "Vector2Int.h"
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace systems {
namespace {
constexpr int kUnreached = -1;
} // namespace

std::optional<std::size_t> DistanceMap::IndexOf(Vector2Int coord) const {
    if ((coord.x < 0) || (coord.x >= width) || (coord.y < 0) || (coord.y >= height)) {
        return std::nullopt;
    }
    return (static_cast<std::size_t>(coord.x) * static_cast<std::size_t>(height)) +
           static_cast<std::size_t>(coord.y);
}

std::optional<int> DistanceMap::At(Vector2Int coord) const {
    const auto index = IndexOf(coord);
    if (!index.has_value() || steps[*index] == kUnreached) {
        return std::nullopt;
    }
    return steps[*index];
}

void DistanceMap::Set(Vector2Int coord, int step) {
    const auto index = IndexOf(coord);
    if (!index.has_value()) {
        throw std::out_of_range("DistanceMap::Set: coordinate outside the map");
    }
    steps[*index] = step;
}

DistanceMap BuildDistanceMap(const domain::Registry &registry, Location goal) {
    const auto &target_map = registry.maps[goal.map_level];
    DistanceMap dmap = {.width = target_map.Width(),
                        .height = target_map.Height(),
                        .steps = std::vector<int>(static_cast<std::size_t>(target_map.Width()) *
                                                      static_cast<std::size_t>(target_map.Height()),
                                                  kUnreached)};

    std::vector<Vector2Int> last_added = {};
    last_added.push_back(goal.coord);
    dmap.Set(goal.coord, 0);
    auto step = 1;

    while (!last_added.empty()) {
        std::vector<Vector2Int> new_added = {};
        for (const Vector2Int &coord : last_added) {
            for (auto cardinal : Vector2Int::Cardinals()) {
                const Location check_loc = {.coord = coord + cardinal, .map_level = goal.map_level};
                if (!dmap.At(check_loc.coord).has_value() && CanEnter(registry, check_loc)) {
                    new_added.push_back(check_loc.coord);
                    dmap.Set(check_loc.coord, step);
                }
            }
        }
        last_added = std::move(new_added);
        step++;
    }

    return dmap;
}
} // namespace systems
