#include "MessageLog.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <iterator>
#include <string>

TEST(MessageLog, PushKeepsInsertionOrder) {
    ui::MessageLog log;
    log.Push("first");
    log.Push("second");

    ASSERT_EQ(std::distance(log.begin(), log.end()), 2);
    auto it = log.begin();
    EXPECT_EQ(*it, "first");
    ++it;
    EXPECT_EQ(*it, "second");
}

TEST(MessageLog, CapsAtMaxLogLinesDroppingOldest) {
    ui::MessageLog log;
    for (int i = 0; i < 10; ++i) {
        log.Push("line " + std::to_string(i));
    }

    ASSERT_EQ(static_cast<std::size_t>(std::distance(log.begin(), log.end())),
              ui::MessageLog::kMaxLogLines);
    auto it = log.begin();
    for (int i = 10 - static_cast<int>(ui::MessageLog::kMaxLogLines); i < 10; ++i, ++it) {
        EXPECT_EQ(*it, "line " + std::to_string(i));
    }
}

TEST(MessageLog, PushAllCapsAcrossTheWholeBatchNotJustPerCall) {
    ui::MessageLog log;
    log.PushAll({"a", "b", "c", "d", "e", "f"});

    ASSERT_EQ(static_cast<std::size_t>(std::distance(log.begin(), log.end())),
              ui::MessageLog::kMaxLogLines);
    auto it = log.begin();
    EXPECT_EQ(*it, "c");
    ++it;
    EXPECT_EQ(*it, "d");
    ++it;
    EXPECT_EQ(*it, "e");
    ++it;
    EXPECT_EQ(*it, "f");
}

TEST(MessageLog, PushAllOnEmptyLogAppendsAllWhenUnderCap) {
    ui::MessageLog log;
    log.PushAll({"only one"});

    ASSERT_EQ(std::distance(log.begin(), log.end()), 1);
    EXPECT_EQ(*log.begin(), "only one");
}
