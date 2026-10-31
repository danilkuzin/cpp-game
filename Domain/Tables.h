#pragma once

#include <array>
#include <cstdint>
#include <utility>

namespace hero {

constexpr int kMaxPlayerLevel = 8;

constexpr std::array<int, 8> kExpLevelTable = {10, 20, 50, 100, 250, 500, 1000, 3000};

constexpr std::array<std::array<int, 8>, 3> kHpLevelTable = {{{10, 20, 30, 50, 80, 130, 210, 340},
                                                              {8, 12, 20, 30, 50, 100, 150, 250},
                                                              {4, 7, 10, 15, 30, 50, 100, 175}}};

constexpr std::array<std::array<int, 8>, 3> kManaLevelTable = {
    {{0, 0, 0, 0, 0, 0, 0, 0},
     {4, 7, 10, 15, 30, 50, 100, 175},
     {8, 12, 20, 30, 50, 100, 150, 250}}};

enum class Skill : std::uint8_t {
    HandWeapon = 0,
    TrapSearch = 1,
    Defence = 2,
    RangedWeapon = 3,
    Spellcasting = 4,
    MaxSkills = Spellcasting,
};

// [race][char]
constexpr std::array<std::array<std::pair<int, int>, 6>, 4> kRaceTable = {
    {{{{12, 4}, {12, 4}, {12, 4}, {12, 4}, {12, 4}, {10, 4}}},
     {{{11, 3}, {16, 2}, {11, 3}, {14, 3}, {13, 3}, {13, 2}}},
     {{{16, 2}, {12, 3}, {16, 2}, {9, 1}, {10, 2}, {7, 3}}},
     {{{10, 2}, {13, 1}, {9, 2}, {16, 2}, {15, 3}, {15, 3}}}}};

// [skill][class]
constexpr std::array<std::array<std::pair<int, int>, 3>, 5> kClassSkillTable = {
    {{{{80, 15}, {50, 20}, {25, 10}}},
     {{{20, 20}, {80, 15}, {60, 20}}},
     {{{70, 20}, {50, 20}, {15, 10}}},
     {{{20, 20}, {80, 20}, {35, 20}}},
     {{{0, 0}, {20, 20}, {45, 25}}}}};

} // namespace hero
