#include "MakeRegistry.h"
#include "Config.h"
#include "Map.h"
#include "Registry.h"

domain::Registry MakeTestRegistry() {
    domain::Registry registry;

    const domain::MapConfig map_config = {
        .width = 10, .height = 10, .max_level = 1, .border_size = 2};
    const map::Map map(map_config.width, map_config.height,
                       map::Cell{.terrain_id = 0 /* grass */, .explored = false});
    registry.maps.push_back(map);

    return registry;
}
