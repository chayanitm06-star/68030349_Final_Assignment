#include "DateTime.hpp"

#include <cstdio>
#include <cstring>

namespace Clock {
namespace {

/**
 * @brief Days since 1970-01-01 for a proleptic Gregorian date.
 * @note Howard Hinnant's `days_from_civil` algorithm - branch free, exact and
 *       valid for the whole range of the DS1307 (2000..2099).
 */
constexpr int64_t daysFromCivil(int64_t y, unsigned m, unsigned d) noexcept {
    y -= (m <= 2) ? 1 : 0;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);              // [0, 399]
    const unsigned doy = (153u * (m + ((m > 2) ? -3u : 9u)) + 2u) / 5u + d - 1u;  // [0, 365]
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;           // [0, 146096]
    return era * 146097LL + static_cast<int64_t>(doe) - 719468LL;
}

constexpr bool isLeapYear(int64_t year) noexcept {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

constexpr uint8_t daysInMonth(int64_t year, uint8_t month) noexcept {
    constexpr uint8_t kDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return kDays[month - 1];
}

}  // namespace

bool DateTime::isValid() const noexcept {
    if (month < 1 || month > 12) {
        return false;
    }
    if (day < 1 || day > daysInMonth(static_cast<int64_t>(year), month)) {
        return false;
    }
    if (dayOfWeek < 1 || dayOfWeek > 7) {
        return false;
    }
    if (hour > 23 || minute > 59 || second > 59) {
        return false;
    }
    return true;
}

DateTime DateTime::fromUnixTime(int64_t unixSeconds, int32_t timeZoneOffsetSeconds) noexcept {
    const int64_t local = unixSeconds + static_cast<int64_t>(timeZoneOffsetSeconds);

    // Floor division so that negative epochs still map to the correct day.
    int64_t days = local / 86400;
    int64_t secondsOfDay = local % 86400;
    if (secondsOfDay < 0) {
        secondsOfDay += 86400;
        days -= 1;
    }

    DateTime out{};
    out.hour = static_cast<uint8_t>(secondsOfDay / 3600);
    out.minute = static_cast<uint8_t>((secondsOfDay % 3600) / 60);
    out.second = static_cast<uint8_t>(secondsOfDay % 60);

    int64_t z = days + 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460u + doe / 36524u - doe / 146096u) / 365u;
    const int64_t y = static_cast<int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365u * yoe + yoe / 4u - yoe / 100u);
    const unsigned mp = (5u * doy + 2u) / 153u;
    const unsigned d = doy - (153u * mp + 2u) / 5u + 1u;
    const unsigned m = mp + ((mp < 10) ? 3u : -9u);

    out.year = static_cast<uint16_t>(y + ((m <= 2) ? 1 : 0));
    out.month = static_cast<uint8_t>(m);
    out.day = static_cast<uint8_t>(d);

    // 1970-01-01 was a Thursday (index 4 with Sunday == 0).
    int64_t wday = (days + 4) % 7;
    if (wday < 0) {
        wday += 7;
    }
    out.dayOfWeek = static_cast<uint8_t>(wday + 1);  // DS1307: 1 = Sunday
    return out;
}

int64_t DateTime::toUnixTime() const noexcept {
    const int64_t days = daysFromCivil(static_cast<int64_t>(year), month, day);
    return days * 86400LL + static_cast<int64_t>(hour) * 3600LL + static_cast<int64_t>(minute) * 60LL +
           static_cast<int64_t>(second);
}

void DateTime::toTm(struct tm& out) const noexcept {
    std::memset(&out, 0, sizeof(out));
    out.tm_year = static_cast<int>(year) - 1900;
    out.tm_mon = static_cast<int>(month) - 1;
    out.tm_mday = static_cast<int>(day);
    out.tm_hour = static_cast<int>(hour);
    out.tm_min = static_cast<int>(minute);
    out.tm_sec = static_cast<int>(second);
    out.tm_wday = static_cast<int>(dayOfWeek) % 7;  // struct tm: 0 = Sunday
    out.tm_yday = 0;
    out.tm_isdst = -1;
}

DateTime DateTime::fromTm(const struct tm& in) noexcept {
    DateTime out{};
    out.year = static_cast<uint16_t>(in.tm_year + 1900);
    out.month = static_cast<uint8_t>(in.tm_mon + 1);
    out.day = static_cast<uint8_t>(in.tm_mday);
    out.hour = static_cast<uint8_t>(in.tm_hour);
    out.minute = static_cast<uint8_t>(in.tm_min);
    out.second = static_cast<uint8_t>(in.tm_sec);
    out.dayOfWeek = static_cast<uint8_t>((in.tm_wday % 7) + 1);
    return out;
}

void DateTime::format(char* buffer, size_t bufferSize) const noexcept {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    std::snprintf(buffer, bufferSize, "%04u-%02u-%02u %02u:%02u:%02u", static_cast<unsigned>(year),
                  static_cast<unsigned>(month), static_cast<unsigned>(day), static_cast<unsigned>(hour),
                  static_cast<unsigned>(minute), static_cast<unsigned>(second));
}

void DateTime::formatDate(char* buffer, size_t bufferSize) const noexcept {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    std::snprintf(buffer, bufferSize, "%04u-%02u-%02u", static_cast<unsigned>(year),
                  static_cast<unsigned>(month), static_cast<unsigned>(day));
}

void DateTime::formatTime(char* buffer, size_t bufferSize) const noexcept {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    std::snprintf(buffer, bufferSize, "%02u:%02u:%02u", static_cast<unsigned>(hour),
                  static_cast<unsigned>(minute), static_cast<unsigned>(second));
}

const char* DateTime::weekdayName() const noexcept {
    static constexpr const char* kNames[8] = {"---", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    if (dayOfWeek < 1 || dayOfWeek > 7) {
        return kNames[0];
    }
    return kNames[dayOfWeek];
}

}  // namespace Clock
