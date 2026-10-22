/**
 * @file TimeManager.hpp
 * @brief Central time authority of the clock.
 *
 * Resolution order of @ref now():
 *   1. POSIX system clock, but only after a successful SNTP synchronisation.
 *   2. DS1307 real time clock (fallback when the network is unavailable).
 *   3. The last successfully read value (degraded fallback).
 */
#pragma once

#include <cstdint>

#include "DateTime.hpp"
#include "Interfaces.hpp"

namespace Clock {

class TimeManager {
public:
    TimeManager(IRtcSource& rtc, ISystemClock& systemClock, INtpClient& ntp);

    /** @brief Local time zone offset in seconds (e.g. 25200 for UTC+7). */
    void setTimeZoneOffset(int32_t offsetSeconds) noexcept;
    [[nodiscard]] int32_t timeZoneOffsetSeconds() const noexcept { return m_timeZoneOffsetSeconds; }

    /** @brief Best available "now". */
    [[nodiscard]] DateTime now() const;

    /** @brief True once an SNTP synchronisation succeeded. */
    [[nodiscard]] bool isNtpSynced() const noexcept { return m_ntpSynced; }

    /**
     * @brief Perform an SNTP synchronisation and push the result into the RTC.
     * @return True when the system time could be refreshed.
     */
    bool requestSync();

    /** @brief Write a manually calibrated time into the RTC. */
    bool applyManualTime(const DateTime& value);

    /** @brief Last value that could be read from any source. */
    [[nodiscard]] const DateTime& lastKnown() const noexcept { return m_lastKnown; }

private:
    IRtcSource& m_rtc;
    ISystemClock& m_systemClock;
    INtpClient& m_ntp;

    int32_t m_timeZoneOffsetSeconds{0};
    bool m_ntpSynced{false};
    mutable DateTime m_lastKnown{};
};

}  // namespace Clock
