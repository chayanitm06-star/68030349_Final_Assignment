/**
 * @file DateTime.hpp
 * @brief Calendar / wall-clock value object shared by the RTC driver, the time
 *        manager and the UI renderer.
 * @details ESP-IDF free by design so that it can be unit-tested on the host.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <ctime>

namespace Clock {

/**
 * @brief Broken down date and time.
 * @note The day-of-week convention follows the DS1307 datasheet (1 = Sunday).
 */
struct DateTime {
    uint16_t year{2000};
    uint8_t month{1};
    uint8_t day{1};
    uint8_t dayOfWeek{1};
    uint8_t hour{0};
    uint8_t minute{0};
    uint8_t second{0};

    /** @brief Validate the calendar fields (month 1..12, day 1..31, ...). */
    [[nodiscard]] bool isValid() const noexcept;

    /**
     * @brief Build a DateTime from a POSIX (UTC) timestamp.
     * @param unixSeconds Seconds since 1970-01-01T00:00:00Z.
     * @param timeZoneOffsetSeconds Offset applied to reach local time.
     */
    [[nodiscard]] static DateTime fromUnixTime(int64_t unixSeconds, int32_t timeZoneOffsetSeconds = 0) noexcept;

    /** @brief Convert the local time back into a POSIX (UTC) timestamp. */
    [[nodiscard]] int64_t toUnixTime() const noexcept;

    /** @brief Fill a standard @c struct tm from this value. */
    void toTm(struct tm& out) const noexcept;

    /** @brief Build a DateTime from a standard @c struct tm. */
    [[nodiscard]] static DateTime fromTm(const struct tm& in) noexcept;

    /** @brief Render as "YYYY-MM-DD HH:MM:SS" (needs at least 20 bytes). */
    void format(char* buffer, size_t bufferSize) const noexcept;

    /** @brief Render the calendar part as "YYYY-MM-DD" (needs at least 11 bytes). */
    void formatDate(char* buffer, size_t bufferSize) const noexcept;

    /** @brief Render the time part as "HH:MM:SS" (needs at least 9 bytes). */
    void formatTime(char* buffer, size_t bufferSize) const noexcept;

    /** @brief Three letter weekday name, e.g. "MON". */
    [[nodiscard]] const char* weekdayName() const noexcept;
};

}  // namespace Clock
