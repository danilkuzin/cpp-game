#pragma once

#include <array>

namespace roll {
struct Dices {
    int count = 0;
    int sides = 0;
};

int Dice(Dices dices);
void Seed(unsigned int seed);
bool SkillCheck(int skill_value, int difficulty);

int Index(int size);
bool Chance(int percent);

const auto kDice100 = 100;

const auto kDefAdvantage = 3;
const auto kIqAdvantage = 3;
const auto kConAdvantage = 5;

const std::array<int, 2> kDiceRoll = {3, 6};

} // namespace roll
