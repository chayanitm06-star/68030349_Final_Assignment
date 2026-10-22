/**
 * @file AlarmEngine.hpp
 * @brief Alarm domain service: comparison, ringing and buzzer control.
 */
#pragma once

#include <cstdint>

#include "AlarmLogic.hpp"
#include "Interfaces.hpp"

namespace Clock {

class AlarmEngine {
public:
    /** @brief Default ringing duration (60 s). */
    static constexpr uint32_t kDefaultRingingTimeoutMs = 60000;

    /** @brief Tone used while the alarm is ringing. */
    static constexpr uint16_t kAlarmToneHz = 2700;

    explicit AlarmEngine(IBuzzer& buzzer, uint32_t ringingTimeoutMs = kDefaultRingingTimeoutMs);

    /** @brief Configure the alarm time and the armed state. */
    void setAlarm(uint8_t hour, uint8_t minute, bool enabled) noexcept;

    /** @brief Arm / disarm the alarm. */
    void setEnabled(bool enabled) noexcept;

    [[nodiscard]] uint8_t hour() const noexcept { return m_logic.hour(); }
    [[nodiscard]] uint8_t minute() const noexcept { return m_logic.minute(); }
    [[nodiscard]] bool isEnabled() const noexcept { return m_logic.enabled(); }
    [[nodiscard]] bool isRinging() const noexcept { return m_ringing; }

    /**
     * @brief Evaluate the alarm against the current wall clock time.
     * @return True when the alarm started to ring at this call.
     */
    bool check(uint8_t hour, uint8_t minute);

    /** @brief Stop the ringing (user pressed LAP / SET). */
    void stopRinging();

    /** @brief Service the ringing timeout. */
    void update(uint32_t nowMs);

    void setRingingTimeoutMs(uint32_t timeoutMs) noexcept { m_ringingTimeoutMs = timeoutMs; }
    [[nodiscard]] uint32_t ringingTimeoutMs() const noexcept { return m_ringingTimeoutMs; }

    /** @brief Number of times the alarm rang since boot (diagnostics). */
    [[nodiscard]] uint32_t triggerCount() const noexcept { return m_triggerCount; }

    /** @brief Raw comparison helper (pure). */
    [[nodiscard]] bool matches(uint8_t hour, uint8_t minute) const noexcept {
        return m_logic.matches(hour, minute);
    }

private:
    IBuzzer& m_buzzer;
    AlarmLogic m_logic;
    uint32_t m_ringingTimeoutMs{kDefaultRingingTimeoutMs};
    bool m_ringing{false};
    uint32_t m_ringingSinceMs{0};
    uint32_t m_triggerCount{0};
};

}  // namespace Clock
