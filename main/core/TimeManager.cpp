#include "TimeManager.hpp"

namespace Clock {

TimeManager::TimeManager(IRtcSource& rtc, ISystemClock& systemClock, INtpClient& ntp)
    : m_rtc(rtc), m_systemClock(systemClock), m_ntp(ntp) {}

void TimeManager::setTimeZoneOffset(int32_t offsetSeconds) noexcept {
    m_timeZoneOffsetSeconds = offsetSeconds;
}

DateTime TimeManager::now() const {
    // 1) Network synchronised system clock.
    if (m_ntpSynced) {
        int64_t unixSeconds = 0;
        if (m_systemClock.readUnixTime(unixSeconds)) {
            const DateTime local = DateTime::fromUnixTime(unixSeconds, m_timeZoneOffsetSeconds);
            m_lastKnown = local;
            return local;
        }
    }

    // 2) Battery backed RTC.
    DateTime rtcTime{};
    if (m_rtc.readDateTime(rtcTime)) {
        m_lastKnown = rtcTime;
        return rtcTime;
    }

    // 3) Last known value - the clock keeps running even without a source.
    return m_lastKnown;
}

bool TimeManager::requestSync() {
    if (!m_ntp.syncTime()) {
        return false;
    }

    int64_t unixSeconds = 0;
    if (!m_systemClock.readUnixTime(unixSeconds)) {
        return false;
    }

    m_ntpSynced = true;

    // Calibrate the DS1307 with the freshly synchronised time so that the
    // clock survives a power cycle without the network.
    const DateTime local = DateTime::fromUnixTime(unixSeconds, m_timeZoneOffsetSeconds);
    m_rtc.writeDateTime(local);
    m_lastKnown = local;
    return true;
}

bool TimeManager::applyManualTime(const DateTime& value) {
    if (!m_rtc.writeDateTime(value)) {
        return false;
    }
    m_lastKnown = value;
    return true;
}

}  // namespace Clock
