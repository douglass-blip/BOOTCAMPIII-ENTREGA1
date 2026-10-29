#include <gtest/gtest.h>

#include "time_utils.hpp"

TEST(TimeUtils, ParsesUtcOffsetsAndFormatsNormalizedUtcTimestamp) {
    const auto parsed = TimeUtils::parseIso8601("2030-01-02T03:04:05.25+02:30");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(
        TimeUtils::formatIso8601(*parsed),
        "2030-01-02T00:34:05.250Z");
}

TEST(TimeUtils, RejectsInvalidDatesAndUnsupportedFractionPrecision) {
    EXPECT_FALSE(TimeUtils::parseIso8601("2030-02-30T00:00:00Z"));
    EXPECT_FALSE(TimeUtils::parseIso8601("2030-01-01T00:00:00"));
    EXPECT_FALSE(TimeUtils::parseIso8601("2030-01-01T00:00:00.1234Z"));
    EXPECT_FALSE(TimeUtils::parseIso8601("2030-01-01T00:00:00+24:00"));
}

TEST(TimeUtils, RoundTripsPreEpochTimes) {
    const auto time = TimeUtils::Clock::time_point(std::chrono::milliseconds(-1));

    EXPECT_EQ(TimeUtils::parseIso8601(TimeUtils::formatIso8601(time)), time);
}
