#pragma once

#include "Location.h"
#include "RectInt.h"
#include "Tables.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace hero {
constexpr int kMaxChars = 6;
constexpr int kChrStr = 0;
constexpr int kChrDex = 1;
constexpr int kChrCon = 2;
constexpr int kChrIq = 3;
constexpr int kChrWis = 4;
constexpr int kChrCha = 5;

constexpr int kVisionRadius = 2;

enum class Race : int8_t {
    Human = 0,
    Elf = 1,
    Dwarf = 2,
    Hobbit = 3,
    MaxRaces = Hobbit,
};

enum class HClass : int8_t {
    Warrior = 0,
    Ranger = 1,
    Mage = 2,
    MaxClasses = Mage,
};

struct HeroSkills {
    std::array<int, static_cast<std::size_t>(Skill::MaxSkills) + 1> skills{};
};

struct HeroExperience {
    int xp = 0;
    int level = 1;
    HClass hclass = HClass::Warrior;
};

RectInt GetVisibleArea(Location location, int vis_long);
} // namespace hero
