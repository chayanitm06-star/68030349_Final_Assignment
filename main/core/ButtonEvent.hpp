/**
 * @file ButtonEvent.hpp
 * @brief Value object describing a debounced button event.
 */
#pragma once

#include <cstdint>

#include "Debounce.hpp"

namespace Clock {

/** @brief Number of buttons on the panel. */
inline constexpr uint8_t kButtonCount = 4;

/** @brief Physical buttons of the clock panel. */
enum class ButtonId : uint8_t {
    Mode = 0,  ///< MODE  - cycle through the display modes.
    Inc = 1,   ///< INC   - increment the selected value / trigger NTP sync.
    Set = 2,   ///< SET   - toggle the alarm / start-pause the stopwatch.
    Lap = 3,   ///< LAP   - record a lap / reset the stopwatch / silence alarm.
};

/** @brief A debounced button event handed over from the driver to the UI task. */
struct ButtonEvent {
    ButtonId id{ButtonId::Mode};
    PressType type{PressType::Short};
    uint32_t timestampMs{0};
};

}  // namespace Clock
