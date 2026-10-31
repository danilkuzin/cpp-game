#pragma once

#include "Config.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include <string>

domain::Config MakeTestConfig();

domain::ViewConfig MakeTestViewConfig();

database::GameDatabase MakeTestDatabase();

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
game_item::ItemTemplate MakeItemTemplate(std::string id, std::string name, int size,
                                         std::string sprite_name);
