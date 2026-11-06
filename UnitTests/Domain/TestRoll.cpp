#include "Roll.h"
#include <gtest/gtest.h>

namespace {
constexpr int kIterations = 200;
} // namespace

TEST(Roll, SkillCheckAlwaysFailsWhenDifficultyMeetsOrExceedsSkill) {
    for (int i = 0; i < kIterations; ++i) {
        EXPECT_FALSE(roll::SkillCheck(50, 50));
        EXPECT_FALSE(roll::SkillCheck(30, 60));
    }
}

TEST(Roll, SkillCheckAlwaysSucceedsWhenSkillExceedsDifficultyByAtLeastAHundred) {
    for (int i = 0; i < kIterations; ++i) {
        EXPECT_TRUE(roll::SkillCheck(150, 10));
        EXPECT_TRUE(roll::SkillCheck(100, 0));
    }
}
