#include "TestData.h"
#include "Config.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Spell.h"
#include <optional>
#include <string>
#include <utility>

domain::Config MakeTestConfig() {
    return domain::Config{
        .map_config = {.width = 10, .height = 10, .max_level = 1, .border_size = 2},
        .num_monsters = 0,
        .seed = std::nullopt};
}

domain::ViewConfig MakeTestViewConfig() {
    return domain::ViewConfig{.camera_width = 4, .camera_height = 4, .scroll_delta = 1};
}

database::GameDatabase MakeTestDatabase() {
    database::GameDatabase db;
    auto copper =
        MakeItemTemplate(game_item::CurrencyItemId(), "Copper", /*size=*/0, "../copper.bmp");
    copper.currency = game_item::CurrencyComponent{.amount = 0};
    db.AddItem(copper);
    db.AddSpell({.id = "fireball",
                 .name = "Fireball",
                 .mana_cost = spell::ManaCostComponent{.amount = 1},
                 .damage = spell::DamageComponent{.amount = 4},
                 .healing = std::nullopt});
    db.AddSpell({.id = "healing_light",
                 .name = "Healing Light",
                 .mana_cost = spell::ManaCostComponent{.amount = 1},
                 .damage = std::nullopt,
                 .healing = spell::HealingComponent{.amount = 4}});
    return db;
}

game_item::ItemTemplate MakeItemTemplate(std::string id, std::string name, int size,
                                         std::string sprite_name) {
    return game_item::ItemTemplate{.id = std::move(id),
                                   .name = std::move(name),
                                   .melee = std::nullopt,
                                   .ranged = std::nullopt,
                                   .armor = std::nullopt,
                                   .ammo = std::nullopt,
                                   .magic = std::nullopt,
                                   .equippable = std::nullopt,
                                   .currency = std::nullopt,
                                   .value = std::nullopt,
                                   .size = size,
                                   .sprite_name = std::move(sprite_name)};
}
