#pragma once

#include "Config.h"
#include "GameDatabase.h"
#include <string>

namespace dataloader {
class DataLoader {
public:
    static database::GameDatabase LoadDatabase(const std::string &path);
    static domain::Config LoadConfig(const std::string &path);
    static domain::ViewConfig LoadViewConfig(const std::string &path);

private:
    static void LoadMonsters(const std::string &path, database::GameDatabase &db);
    static void LoadItems(const std::string &path, database::GameDatabase &db);
    static void LoadSpells(const std::string &path, database::GameDatabase &db);
};
} // namespace dataloader
