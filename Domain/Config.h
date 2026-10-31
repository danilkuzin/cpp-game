#pragma once

#include <optional>

namespace domain {

struct MapConfig {
    int width = 0;
    int height = 0;
    int max_level = 0;
    int border_size = 0;
};

struct Config {
    MapConfig map_config{};
    int num_monsters = 0;
    // Unset means startup picks a random seed and prints it, so any run can be replayed after
    // the fact.
    std::optional<unsigned int> seed;
};

struct ViewConfig {
    int camera_width = 0;
    int camera_height = 0;
    int scroll_delta = 0;
};

} // namespace domain
