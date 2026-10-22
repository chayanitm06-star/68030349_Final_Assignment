/**
 * @file EspTimerSource.hpp
 * @brief Monotonic millisecond clock based on the ESP-IDF esp_timer driver.
 */
#pragma once

#include <cstdint>

#include "Interfaces.hpp"

namespace Clock {

class EspTimerSource : public ITimeSource {
public:
    /** @brief Microseconds since boot (esp_timer resolution). */
    [[nodiscard]] static int64_t nowUs() { return esp_timer_get_time(); }

    /** @brief Milliseconds since boot. */
    [[nodiscard]] uint64_t nowMs() const override {
        return static_cast<uint64_t>(esp_timer_get_time() / 1000);
    }
};

}  // namespace Clock
