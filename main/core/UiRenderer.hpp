/**
 * @file UiRenderer.hpp
 * @brief Draws the four display modes onto an abstract display surface.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "AlarmEngine.hpp"
#include "Interfaces.hpp"
#include "StopwatchEngine.hpp"
#include "TimeManager.hpp"
#include "UiStateMachine.hpp"

namespace Clock {

class UiRenderer {
public:
    UiRenderer(IDisplay& display, TimeManager& time, AlarmEngine& alarm, StopwatchEngine& stopwatch,
               UiStateMachine& ui);

    /** @brief Draw the complete frame for the currently active mode. */
    void render();

private:
    void renderHeader();
    void renderClock();
    void renderAlarmSet();
    void renderStopwatch();
    void renderManualTime();

    /** @brief Format milliseconds as "MM:SS.CC". */
    static void formatStopwatch(uint64_t milliseconds, char* buffer, size_t bufferSize);

    IDisplay& m_display;
    TimeManager& m_time;
    AlarmEngine& m_alarm;
    StopwatchEngine& m_stopwatch;
    UiStateMachine& m_ui;
};

}  // namespace Clock
