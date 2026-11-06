#pragma once

#include "Location.h"
#include "Registry.h"
#include "Vector2Int.h"
#include <cstddef>
#include <optional>
#include <vector>

namespace systems {

struct DistanceMap {
    int width = 0;
    int height = 0;
    std::vector<int> steps;
    [[nodiscard]] std::optional<int> At(Vector2Int coord) const;
    void Set(Vector2Int coord, int step);

private:
    [[nodiscard]] std::optional<std::size_t> IndexOf(Vector2Int coord) const;
};

DistanceMap BuildDistanceMap(const domain::Registry &registry, Location goal);
} // namespace systems
