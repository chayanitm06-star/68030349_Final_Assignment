/**
 * @file AlarmManager.hpp
 * @brief Application level alarm service (dashboard compatible API).
 *
 * The class glues the pure @ref Clock::AlarmEngine to the LEDC piezo buzzer
 * driver and adds the logging that the reference implementation shows.
 */
#pragma once

#include <cstdint>

#include "AlarmEngine.hpp"
#include "Buzzer.hpp"
#include "driver/gpio.h"
#include "esp_err.h"

namespace Clock {

class AlarmManager {
public:
    explicit AlarmManager(gpio_num_t buzzerPin,
                          uint32_t ringingTimeoutMs = AlarmEngine::kDefaultRingingTimeoutMs);

    /** @brief Configure the buzzer output. */
    esp_err_t init();

    /** @brief Configure the alarm time and the armed state. */
    void setAlarm(int hour, int minute, bool enable);

    /** @brief Compare the current wall clock against the alarm. */
    void checkAlarm(int currentHour, int currentMinute);

    /** @brief Silence a ringing alarm. */
    void stopRinging();

    /** @brief Service the ringing timeout (called by the alarm task). */
    void update(uint32_t nowMs);

    [[nodiscard]] bool isRinging() const;
    [[nodiscard]] bool isEnabled() const;
    [[nodiscard]] int alarmHour() const;
    [[nodiscard]] int alarmMinute() const;
    [[nodiscard]] uint32_t triggerCount() const;

    /** @brief Access to the underlying domain service. */
    [[nodiscard]] AlarmEngine& engine() noexcept { return m_engine; }
    [[nodiscard]] const AlarmEngine& engine() const noexcept { return m_engine; }

private:
    Buzzer m_buzzer;
    AlarmEngine m_engine;
};

}  // namespace Clock
