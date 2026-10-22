#include "AlarmLogic.hpp"

namespace Clock {

void AlarmLogic::setAlarm(uint8_t hour, uint8_t minute, bool enabled) noexcept {
    m_hour = static_cast<uint8_t>(hour % 24u);
    m_minute = static_cast<uint8_t>(minute % 60u);
    m_enabled = enabled;
    m_triggered = false;
}

void AlarmLogic::setEnabled(bool enabled) noexcept {
    m_enabled = enabled;
    if (!enabled) {
        m_triggered = false;
    }
}

bool AlarmLogic::matches(uint8_t hour, uint8_t minute) const noexcept {
    return m_enabled && hour == m_hour && minute == m_minute;
}

bool AlarmLogic::shouldTrigger(uint8_t hour, uint8_t minute) noexcept {
    if (!matches(hour, minute)) {
        return false;
    }
    if (m_triggered) {
        return false;
    }
    m_triggered = true;
    return true;
}

void AlarmLogic::clearTrigger() noexcept {
    m_triggered = false;
}

}  // namespace Clock
