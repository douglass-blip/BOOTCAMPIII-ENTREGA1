#include "time_utils.hpp"

#include <array>
#include <iomanip>
#include <sstream>

namespace {

bool readDigits(
    const std::string& value,
    std::size_t offset,
    std::size_t count,
    int& result
) {
    result = 0;
    if (offset + count > value.size()) {
        return false;
    }
    for (std::size_t i = offset; i < offset + count; ++i) {
        const char character = value[i];
        if (character < '0' || character > '9') {
            return false;
        }
        result = result * 10 + (character - '0');
    }
    return true;
}

bool isLeapYear(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

std::int64_t daysFromCivil(int year, unsigned month, unsigned day) {
    year -= month <= 2;
    const std::int64_t era = (year >= 0 ? year : year - 399) / 400;
    const unsigned year_of_era = static_cast<unsigned>(year - era * 400);
    const unsigned adjusted_month = static_cast<unsigned>(
        static_cast<int>(month) + (month > 2 ? -3 : 9));
    const unsigned day_of_year =
        (153 * adjusted_month + 2) / 5 + day - 1;
    const unsigned day_of_era = year_of_era * 365 + year_of_era / 4
        - year_of_era / 100 + day_of_year;
    return era * 146097 + static_cast<std::int64_t>(day_of_era) - 719468;
}

std::array<int, 3> civilFromDays(std::int64_t days) {
    days += 719468;
    const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const unsigned day_of_era = static_cast<unsigned>(days - era * 146097);
    const unsigned year_of_era = (day_of_era - day_of_era / 1460
        + day_of_era / 36524 - day_of_era / 146096) / 365;
    int year = static_cast<int>(year_of_era) + static_cast<int>(era * 400);
    const unsigned day_of_year = day_of_era
        - (365 * year_of_era + year_of_era / 4 - year_of_era / 100);
    const unsigned month_part = (5 * day_of_year + 2) / 153;
    const unsigned day = day_of_year - (153 * month_part + 2) / 5 + 1;
    const unsigned month = static_cast<unsigned>(
        static_cast<int>(month_part) + (month_part < 10 ? 3 : -9));
    year += month <= 2;
    return {year, static_cast<int>(month), static_cast<int>(day)};
}

} // namespace

std::int64_t TimeUtils::toEpochMilliseconds(Clock::time_point time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()).count();
}

TimeUtils::Clock::time_point TimeUtils::fromEpochMilliseconds(
    std::int64_t milliseconds
) {
    return Clock::time_point(std::chrono::duration_cast<Clock::duration>(
        std::chrono::milliseconds(milliseconds)));
}

std::optional<TimeUtils::Clock::time_point> TimeUtils::parseIso8601(
    const std::string& value
) {
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (value.size() < 20 || value[4] != '-' || value[7] != '-' ||
        (value[10] != 'T' && value[10] != 't') || value[13] != ':' ||
        value[16] != ':' || !readDigits(value, 0, 4, year) ||
        !readDigits(value, 5, 2, month) || !readDigits(value, 8, 2, day) ||
        !readDigits(value, 11, 2, hour) || !readDigits(value, 14, 2, minute) ||
        !readDigits(value, 17, 2, second)) {
        return std::nullopt;
    }

    static constexpr int days_per_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (month < 1 || month > 12 || day < 1 ||
        day > days_per_month[month - 1] + (month == 2 && isLeapYear(year)) ||
        hour > 23 || minute > 59 || second > 59) {
        return std::nullopt;
    }

    std::size_t position = 19;
    int milliseconds = 0;
    if (position < value.size() && value[position] == '.') {
        ++position;
        std::size_t fraction_digits = 0;
        while (position < value.size() && value[position] >= '0' &&
               value[position] <= '9') {
            if (fraction_digits >= 3) {
                return std::nullopt;
            }
            milliseconds = milliseconds * 10 + (value[position] - '0');
            ++position;
            ++fraction_digits;
        }
        if (fraction_digits == 0) {
            return std::nullopt;
        }
        while (fraction_digits < 3) {
            milliseconds *= 10;
            ++fraction_digits;
        }
    }

    int offset_minutes = 0;
    if (position < value.size() && (value[position] == 'Z' || value[position] == 'z')) {
        ++position;
    } else if (position + 6 == value.size() &&
               (value[position] == '+' || value[position] == '-') &&
               value[position + 3] == ':') {
        int offset_hour = 0;
        int offset_minute = 0;
        if (!readDigits(value, position + 1, 2, offset_hour) ||
            !readDigits(value, position + 4, 2, offset_minute) ||
            offset_hour > 23 || offset_minute > 59) {
            return std::nullopt;
        }
        offset_minutes = offset_hour * 60 + offset_minute;
        if (value[position] == '-') {
            offset_minutes = -offset_minutes;
        }
        position += 6;
    } else {
        return std::nullopt;
    }
    if (position != value.size()) {
        return std::nullopt;
    }

    const std::int64_t days = daysFromCivil(
        year, static_cast<unsigned>(month), static_cast<unsigned>(day));
    const std::int64_t local_seconds = days * 86400 + hour * 3600
        + minute * 60 + second;
    const std::int64_t utc_milliseconds =
        (local_seconds - offset_minutes * 60) * 1000 + milliseconds;
    return fromEpochMilliseconds(utc_milliseconds);
}

std::string TimeUtils::formatIso8601(Clock::time_point time) {
    const std::int64_t total_milliseconds = toEpochMilliseconds(time);
    std::int64_t seconds = total_milliseconds / 1000;
    int milliseconds = static_cast<int>(total_milliseconds % 1000);
    if (milliseconds < 0) {
        milliseconds += 1000;
        --seconds;
    }

    std::int64_t days = seconds / 86400;
    std::int64_t seconds_in_day = seconds % 86400;
    if (seconds_in_day < 0) {
        seconds_in_day += 86400;
        --days;
    }
    const auto date = civilFromDays(days);
    const int hour = static_cast<int>(seconds_in_day / 3600);
    const int minute = static_cast<int>((seconds_in_day % 3600) / 60);
    const int second = static_cast<int>(seconds_in_day % 60);

    std::ostringstream result;
    result << std::setfill('0') << std::setw(4) << date[0] << '-'
           << std::setw(2) << date[1] << '-' << std::setw(2) << date[2] << 'T'
           << std::setw(2) << hour << ':' << std::setw(2) << minute << ':'
           << std::setw(2) << second;
    if (milliseconds != 0) {
        result << '.' << std::setw(3) << milliseconds;
    }
    result << 'Z';
    return result.str();
}
