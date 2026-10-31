#include "Roll.h"
#include <gtest/gtest.h>

namespace {
constexpr int kIterations = 200;
} // namespace

TEST(Roll, SkillCheckAlwaysFailsWhenDifficultyMeetsOrExceedsSkill) {
    for (int i = 0; i < kIterations; ++i) {
        EXPECT_FALSE(roll::SkillCheck(/*skill_value=*/50, /*difficulty=*/50));
        EXPECT_FALSE(roll::SkillCheck(/*skill_value=*/30, /*difficulty=*/60));
    }
}

TEST(Roll, SkillCheckAlwaysSucceedsWhenSkillExceedsDifficultyByAtLeastAHundred) {
    for (int i = 0; i < kIterations; ++i) {
        EXPECT_TRUE(roll::SkillCheck(/*skill_value=*/150, /*difficulty=*/10));
        EXPECT_TRUE(roll::SkillCheck(/*skill_value=*/100, /*difficulty=*/0));
    }
}
