#include "AlarmEngine.hpp"

namespace Clock {

AlarmEngine::AlarmEngine(IBuzzer& buzzer, uint32_t ringingTimeoutMs)
    : m_buzzer(buzzer), m_ringingTimeoutMs(ringingTimeoutMs) {}

void AlarmEngine::setAlarm(uint8_t hour, uint8_t minute, bool enabled) noexcept {
    m_logic.setAlarm(hour, minute, enabled);
    if (!enabled) {
        stopRinging();
    }
}

void AlarmEngine::setEnabled(bool enabled) noexcept {
    m_logic.setEnabled(enabled);
    if (!enabled) {
        stopRinging();
    }
}

bool AlarmEngine::check(uint8_t hour, uint8_t minute) {
    if (!m_logic.shouldTrigger(hour, minute)) {
        return false;
    }

    m_ringing = true;
    m_ringingSinceMs = 0;
    ++m_triggerCount;
    m_buzzer.setTone(kAlarmToneHz, 50);
    return true;
}

void AlarmEngine::stopRinging() {
    if (m_ringing) {
        m_buzzer.stop();
    }
    m_ringing = false;
    m_logic.clearTrigger();
}

void AlarmEngine::update(uint32_t nowMs) {
    if (!m_ringing) {
        return;
    }
    if (m_ringingSinceMs == 0) {
        m_ringingSinceMs = nowMs;
        return;
    }
    if ((nowMs - m_ringingSinceMs) >= m_ringingTimeoutMs) {
        m_buzzer.stop();
        m_ringing = false;
        m_logic.clearTrigger();
    }
}

}  // namespace Clock
