#include "Roll.h"
#include <algorithm>
#include <random>

namespace {

std::mt19937 &Rng() {
    static std::mt19937 gen = [] {
        std::random_device rd;
        return std::mt19937{rd()};
    }();
    return gen;
}
} // namespace

namespace roll {
[[maybe_unused]] void Seed(unsigned int seed) { Rng().seed(seed); }

int Dice(Dices dices) {
    std::uniform_int_distribution<int> dist(1, dices.sides);
    int sum = 0;
    for (int i = 0; i < dices.count; ++i) {
        sum += dist(Rng());
    }
    return sum;
}

int Index(int size) {
    if (size <= 0) {
        return 0;
    }
    return Dice({.count = 1, .sides = size}) - 1;
}

bool Chance(int percent) { return Dice({.count = 1, .sides = roll::kDice100}) <= percent; }

bool SkillCheck(int skill_value, int difficulty) {
    if (difficulty != 0) {
        skill_value -= difficulty;
        skill_value = std::max(skill_value, 0);
    }

    return roll::Dice({.count = 1, .sides = roll::kDice100}) <= skill_value;
}
} // namespace roll
