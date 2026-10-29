#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace TimeUtils {

using Clock = std::chrono::system_clock;

std::int64_t toEpochMilliseconds(Clock::time_point time);
Clock::time_point fromEpochMilliseconds(std::int64_t milliseconds);
std::optional<Clock::time_point> parseIso8601(const std::string& value);
std::string formatIso8601(Clock::time_point time);

} // namespace TimeUtils
