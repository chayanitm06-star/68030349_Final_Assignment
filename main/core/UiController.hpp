/**
 * @file UiController.hpp
 * @brief Translates debounced button events into domain commands.
 * @details Implements the exact interaction model of the HTML simulators that
 *          ship with the assignment (MODE / INC / SET / LAP).
 */
#pragma once

#include "AlarmEngine.hpp"
#include "ButtonEvent.hpp"
#include "StopwatchEngine.hpp"
#include "TimeManager.hpp"
#include "UiStateMachine.hpp"

namespace Clock {

class UiController {
public:
    UiController(UiStateMachine& ui, AlarmEngine& alarm, StopwatchEngine& stopwatch, TimeManager& time);

    /** @brief React on a single button event. */
    void handleEvent(const ButtonEvent& event);

    /** @brief Number of processed events (diagnostics / unit tests). */
    [[nodiscard]] uint32_t handledEvents() const noexcept { return m_handledEvents; }

private:
    void handleMode(const ButtonEvent& event);
    void handleInc(const ButtonEvent& event);
    void handleSet(const ButtonEvent& event);
    void handleLap(const ButtonEvent& event);

    UiStateMachine& m_ui;
    AlarmEngine& m_alarm;
    StopwatchEngine& m_stopwatch;
    TimeManager& m_time;
    uint32_t m_handledEvents{0};
};

}  // namespace Clock
