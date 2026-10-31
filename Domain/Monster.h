#pragma once
#include "Hero.h"
#include <array>
#include <string>

namespace monster {

struct MonsterPrefab {
    std::string id;
    std::string name;
    int map_level = 0;
    int view_zone = 0;
    int max_hp = 0;
    std::array<int, hero::kMaxChars> chars{};
    std::string tile;
    bool shopkeeper = false;
};

} // namespace monster
