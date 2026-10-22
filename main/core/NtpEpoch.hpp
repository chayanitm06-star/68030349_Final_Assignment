/**
 * @file NtpEpoch.hpp
 * @brief Conversion helpers between NTP timestamps, POSIX epochs and the
 *        configured local time zone offset.
 */
#pragma once

#include <cstdint>

namespace Clock {

/** @brief Seconds between the NTP epoch (1900-01-01) and the Unix epoch (1970-01-01). */
inline constexpr uint64_t kNtpToUnixEpochDelta = 2208988800ULL;

/**
 * @brief Convert a 64 bit NTP timestamp (32.32 fixed point) into Unix seconds.
 */
[[nodiscard]] constexpr int64_t ntpToUnixSeconds(uint64_t ntpTimestamp) noexcept {
    return static_cast<int64_t>(ntpTimestamp >> 32) - static_cast<int64_t>(kNtpToUnixEpochDelta);
}

/**
 * @brief Convert Unix seconds into a 64 bit NTP timestamp (32.32 fixed point).
 */
[[nodiscard]] constexpr uint64_t unixToNtpTimestamp(int64_t unixSeconds) noexcept {
    return (static_cast<uint64_t>(unixSeconds + static_cast<int64_t>(kNtpToUnixEpochDelta)) << 32);
}

/**
 * @brief Apply a time zone offset to a UTC timestamp.
 * @param utcSeconds        Seconds since 1970-01-01T00:00:00Z.
 * @param offsetSeconds     Offset in seconds (e.g. 25200 for UTC+7).
 */
[[nodiscard]] constexpr int64_t applyTimeZoneOffset(int64_t utcSeconds, int32_t offsetSeconds) noexcept {
    return utcSeconds + static_cast<int64_t>(offsetSeconds);
}

}  // namespace Clock
