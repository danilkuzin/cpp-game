#include "DataLoader.h"
#include "GameItem.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

namespace {
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void WriteFile(const std::string &path, const std::string &contents) {
    std::ofstream file(path);
    file << contents;
}
} // namespace

TEST(DataLoader, LoadDatabaseReadsTheMonsterPrefabsFromDisk) {
    auto db = dataloader::DataLoader::LoadDatabase("Data");
    bool found_rat = false;
    for (const auto &[proto_id, monster_prefab] : db.GetMonsters()) {
        if (monster_prefab.name == "Rat") {
            found_rat = true;
        }
    }
    ASSERT_TRUE(found_rat);
}

TEST(DataLoader, LoadsFireballSpell) {
    auto db = dataloader::DataLoader::LoadDatabase("Data");

    const auto &fireball = db.GetSpell("fireball");
    ASSERT_TRUE(fireball.mana_cost.has_value());
    EXPECT_EQ(fireball.mana_cost->amount, 1); // NOLINT(bugprone-unchecked-optional-access)
    ASSERT_TRUE(fireball.damage.has_value());
    EXPECT_EQ(fireball.damage->amount, 4); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_FALSE(fireball.healing.has_value());
}

TEST(DataLoader, LoadsHealingLightSpell) {
    auto db = dataloader::DataLoader::LoadDatabase("Data");

    const auto &healing_light = db.GetSpell("healing_light");
    ASSERT_TRUE(healing_light.mana_cost.has_value());
    EXPECT_EQ(healing_light.mana_cost->amount, 1); // NOLINT(bugprone-unchecked-optional-access)
    ASSERT_TRUE(healing_light.healing.has_value());
    EXPECT_EQ(healing_light.healing->amount, 4); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_FALSE(healing_light.damage.has_value());
}

TEST(GameDatabase, UnknownSpellIdThrows) {
    const database::GameDatabase db;
    EXPECT_THROW((void)db.GetSpell("no_such_spell"), std::out_of_range);
}

TEST(GameDatabase, UnknownItemIdThrows) {
    const database::GameDatabase db;
    EXPECT_THROW((void)db.GetItem("no_such_item"), std::out_of_range);
}

TEST(DataLoader, LoadedDatabaseHasTheCurrencyTemplateCodeAssumes) {
    auto db = dataloader::DataLoader::LoadDatabase("Data");

    const auto &copper = db.GetItem(game_item::CurrencyItemId());
    EXPECT_TRUE(copper.currency.has_value());
}

TEST(DataLoader, LoadConfigParsesEveryFieldAndLeavesSeedUnsetWhenAbsent) {
    const std::string path = "test_config_no_seed.json";
    WriteFile(path, R"({
        "map": {"width": 20, "height": 30, "max_level": 2, "border_size": 3},
        "camera": {"width": 6, "height": 7, "scroll_delta": 2},
        "monsters": {"start_num": 5}
    })");

    auto config = dataloader::DataLoader::LoadConfig(path);
    std::remove(path.c_str());

    EXPECT_EQ(config.map_config.width, 20);
    EXPECT_EQ(config.map_config.height, 30);
    EXPECT_EQ(config.map_config.max_level, 2);
    EXPECT_EQ(config.map_config.border_size, 3);
    EXPECT_EQ(config.num_monsters, 5);
    EXPECT_FALSE(config.seed.has_value());
}

TEST(DataLoader, LoadViewConfigParsesEveryField) {
    const std::string path = "test_view_config.json";
    WriteFile(path, R"({
        "camera": {"width": 6, "height": 7, "scroll_delta": 2}
    })");

    auto view = dataloader::DataLoader::LoadViewConfig(path);
    std::remove(path.c_str());

    EXPECT_EQ(view.camera_width, 6);
    EXPECT_EQ(view.camera_height, 7);
    EXPECT_EQ(view.scroll_delta, 2);
}

TEST(DataLoader, LoadConfigParsesSeedWhenPresent) {
    const std::string path = "test_config_with_seed.json";
    WriteFile(path, R"({
        "map": {"width": 20, "height": 30, "max_level": 2, "border_size": 3},
        "camera": {"width": 6, "height": 7, "scroll_delta": 2},
        "monsters": {"start_num": 5},
        "seed": 20260805
    })");

    auto config = dataloader::DataLoader::LoadConfig(path);
    std::remove(path.c_str());

    ASSERT_TRUE(config.seed.has_value());
    EXPECT_EQ(config.seed.value(), 20260805U); // NOLINT(bugprone-unchecked-optional-access)
}
