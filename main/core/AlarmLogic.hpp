/**
 * @file AlarmLogic.hpp
 * @brief Pure alarm comparison / latching logic (no hardware access).
 */
#pragma once

#include <cstdint>

namespace Clock {

/**
 * @brief Compares the current wall clock against the configured alarm.
 *
 * The logic latches the trigger so that a single alarm minute produces exactly
 * one ring event even though the alarm task runs every second.
 */
class AlarmLogic {
public:
    /** @brief Configure the alarm. */
    void setAlarm(uint8_t hour, uint8_t minute, bool enabled) noexcept;

    /** @brief Enable / disable the alarm without changing its time. */
    void setEnabled(bool enabled) noexcept;

    [[nodiscard]] uint8_t hour() const noexcept { return m_hour; }
    [[nodiscard]] uint8_t minute() const noexcept { return m_minute; }
    [[nodiscard]] bool enabled() const noexcept { return m_enabled; }

    /** @brief Pure comparison: does @p hour / @p minute match the alarm? */
    [[nodiscard]] bool matches(uint8_t hour, uint8_t minute) const noexcept;

    /**
     * @brief Evaluate the alarm for the given wall clock time.
     * @return True exactly once per matching minute (the trigger is latched).
     */
    bool shouldTrigger(uint8_t hour, uint8_t minute) noexcept;

    /** @brief Release the latch so the alarm may fire again. */
    void clearTrigger() noexcept;

private:
    uint8_t m_hour{7};
    uint8_t m_minute{0};
    bool m_enabled{false};
    bool m_triggered{false};
};

}  // namespace Clock
