/**
 * @file Buzzer.hpp
 * @brief LEDC based PWM driver for the passive piezo buzzer.
 */
#pragma once

#include <cstdint>

#include "Interfaces.hpp"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

namespace Clock {

class Buzzer : public IBuzzer {
public:
    /** @brief Duty resolution of the LEDC timer (10 bit -> 0..1023). */
    static constexpr uint32_t kDutyResolutionBits = 10;

    explicit Buzzer(gpio_num_t pin, ledc_channel_t channel = LEDC_CHANNEL_0,
                    ledc_timer_t timer = LEDC_TIMER_0);

    /** @brief Configure the LEDC timer and the output channel. */
    esp_err_t init();

    /** @brief Start a tone with the given frequency and duty cycle. */
    void setTone(uint16_t frequencyHz, uint8_t dutyPercent) override;

    /** @brief Stop the tone (output goes to the idle level). */
    void stop() override;

    [[nodiscard]] bool isActive() const override { return m_active; }

private:
    gpio_num_t m_pin;
    ledc_channel_t m_channel;
    ledc_timer_t m_timer;
    bool m_initialised{false};
    bool m_active{false};
};

}  // namespace Clock
